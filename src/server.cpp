//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>
#include <thread>
#include <thread>
#include <condition_variable>

#include "expenses.hpp"
#include "earnings.hpp"
#include "accounts.hpp"
#include "incomes.hpp"
#include "assets.hpp"
#include "liabilities.hpp"
#include "config.hpp"
#include "objectives.hpp"
#include "wishes.hpp"
#include "fortune.hpp"
#include "recurring.hpp"
#include "debts.hpp"
#include "currency.hpp"
#include "share.hpp"
#include "http.hpp"
#include "logging.hpp"

#include "api/server_api.hpp"
#include "pages/server_pages.hpp"

using namespace budget;

namespace {

httplib::Server * server_ptr = nullptr;
volatile bool cron = true;

std::mutex lock;
std::condition_variable cv;

void server_signal_handler(int signum) {
    LOG_F(INFO, "Received signal ({})", signum);

    cron = false;

    if (server_ptr) {
        server_ptr->stop();
    }

    cv.notify_all();
}

void install_signal_handler() {
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = server_signal_handler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    LOG_F(INFO, "Installed the signal handler");
}

bool start_server(){
    // Name the thread
    pthread_t self = pthread_self();
    pthread_setname_np(self, "server thread");
    loguru::set_thread_name("server thread");

    LOG_F(INFO, "Started the server thread");

    httplib::Server server;

    load_pages(server);
    load_api(server);

    install_signal_handler();

    auto port = get_server_port();
    auto listen = get_server_listen();
    server_ptr = &server;

    // Listen
    LOG_F(INFO, "Server is starting to listen on {}:{}", listen, port);
    if (!server.listen(listen.c_str(), port)) {
        LOG_F(ERROR, "Server failed to start");
        return false;
    }

    LOG_F(INFO, "Server has exited normally");
    return true;
}

void start_cron_loop(){
    // Name the thread
    pthread_t self = pthread_self();
    pthread_setname_np(self, "cron thread");
    loguru::set_thread_name("cron thread");

    LOG_F(INFO, "Started the cron thread");
    size_t hours = 0;

    while(cron){
        using namespace std::chrono_literals;

        {
            std::unique_lock<std::mutex> lk(lock);
            cv.wait_for(lk, 1h);

            if (!cron) {
                break;
            }
        }

        ++hours;

        check_for_recurrings();

        // We save the cache once per day
        if (hours % 24 == 0) {
            save_currency_cache();
            save_share_price_cache();
        }

        // Every four hours, we refresh the currency cache
        // Only current day rates are refreshed
        if (hours % 4 == 0) {
            std::cout << "Refresh the currency cache" << std::endl;
            budget::refresh_currency_cache();
        }

        // Every hour, we try to prefetch value for new days
        std::cout << "Prefetch the share cache" << std::endl;
        budget::prefetch_share_price_cache();
    }

    LOG_F(INFO, "Cron thread has exited");
}

void load(){
    load_accounts();
    load_incomes();
    load_expenses();
    load_earnings();
    load_assets();
    load_liabilities();
    load_objectives();
    load_wishes();
    load_fortunes();
    load_recurrings();
    load_debts();
    load_wishes();
}

} //end of anonymous namespace

int main(int argc, char** argv){
    std::locale global_locale("");
    std::locale::global(global_locale);

    // Initialize logging for the server
    loguru::g_stderr_verbosity = loguru::Verbosity_INFO;
    loguru::init(argc, argv);

    if (!load_config()) {
        LOG_F(ERROR, "Could not config");
        return 0;
    }

    set_server_running();

    // Restore the caches
    load_currency_cache();
    load_share_price_cache();

    auto old_data_version = to_number<size_t>(internal_config_value("data_version"));

    LOG_F(INFO, "Detected database version {}", old_data_version);

    if (old_data_version > DATA_VERSION) {
        LOG_F(ERROR, "Unsupported database version ({}), you should update budgetwarrior", old_data_version);

        return 0;
    }

    if (old_data_version < MIN_DATA_VERSION) {
        LOG_F(ERROR, "Your database version ({}) is not supported anymore", old_data_version);
        LOG_F(ERROR, "You can use an older version of budgetwarrior to migrate it");

        return 0;
    }

    if (old_data_version < DATA_VERSION) {
        LOG_F(INFO, "Migrating database to version {}...", DATA_VERSION);

        try {
            if (old_data_version <= 4 && DATA_VERSION >= 5) {
                migrate_assets_4_to_5();
            }

            if (old_data_version <= 5 && DATA_VERSION >= 6) {
                migrate_assets_5_to_6();
            }

            if (old_data_version <= 6 && DATA_VERSION >= 7) {
                migrate_liabilities_6_to_7();
            }

            if (old_data_version <= 7 && DATA_VERSION >= 8) {
                migrate_assets_7_to_8();
            }
        } catch (const budget_exception& e) {
            LOG_F(ERROR, "budget_exception occured in migrate: {}", e.message());
            return 0;
        } catch (const date_exception& e) {
            LOG_F(ERROR, "date_exception occured in migrate: {}", e.message());
            return 0;
        } catch (const std::exception& e) {
            LOG_F(ERROR, "std::exception occured in migrate: {}", e.what());
            return 0;
        }

        internal_config_set("data_version", to_string(DATA_VERSION));

        // We want to make sure the new data version is set in stone!
        save_config();

        LOG_F(INFO, "Migrated to database version {}", DATA_VERSION);
    }

    // Load all the data
    try {
        load();
    } catch (const budget_exception& e) {
        LOG_F(ERROR, "budget_exception occured in load: {}", e.message());
        return 0;
    } catch (const date_exception& e) {
        LOG_F(ERROR, "date_exception occured in load: {}", e.message());
        return 0;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "std::exception occured in load: {}", e.what());
        return 0;
    }

    volatile bool success = false;
    std::thread server_thread([&success](){ success = start_server(); });
    std::thread cron_thread([](){ start_cron_loop(); });

    server_thread.join();

    if (!success) {
        cron = false;
    }

    cron_thread.join();

    // Save the caches
    save_currency_cache();
    save_share_price_cache();

    save_config();

    return 0;
}
