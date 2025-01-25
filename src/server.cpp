//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <condition_variable>
#include <set>
#include <thread>

#include "accounts.hpp"
#include "api/server_api.hpp"
#include "assets.hpp"
#include "config.hpp"
#include "currency.hpp"
#include "data.hpp"
#include "debts.hpp"
#include "earnings.hpp"
#include "expenses.hpp"
#include "fortune.hpp"
#include "http.hpp"
#include "incomes.hpp"
#include "liabilities.hpp"
#include "logging.hpp"
#include "objectives.hpp"
#include "pages/server_pages.hpp"
#include "recurring.hpp"
#include "share.hpp"
#include "wishes.hpp"

using namespace budget;

namespace {

httplib::Server* server_ptr = nullptr;
std::atomic<bool>    cron       = true;

std::mutex              lock;
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
    struct sigaction action {};
    sigemptyset(&action.sa_mask);
    action.sa_flags   = 0;
    action.sa_handler = server_signal_handler;
    sigaction(SIGTERM, &action, nullptr);
    sigaction(SIGINT, &action, nullptr);

    LOG_F(INFO, "Installed the signal handler");
}

bool start_server() {
    // Name the thread
    const pthread_t self = pthread_self();
    pthread_setname_np(self, "server thread");
    loguru::set_thread_name("server thread");

    LOG_F(INFO, "Started the server thread");

    httplib::Server server;

    load_pages(server);
    load_api(server);

    install_signal_handler();

    auto port   = get_server_port();
    auto listen = get_server_listen();
    server_ptr  = &server;

    // Listen
    LOG_F(INFO, "Server is starting to listen on {}:{}", listen, port);
    if (!server.listen(listen.c_str(), port)) {
        LOG_F(ERROR, "Server failed to start");
        return false;
    }

    LOG_F(INFO, "Server has exited normally");
    return true;
}

void start_cron_loop() {
    // Name the thread
    const pthread_t self = pthread_self();
    pthread_setname_np(self, "cron thread");
    loguru::set_thread_name("cron thread");

    LOG_F(INFO, "cron: Started the cron thread");
    size_t hours = 0;

    while (cron) {
        using namespace std::chrono_literals;

        {
            std::unique_lock lk(lock);
            cv.wait_for(lk, 1h);

            if (!cron) {
                break;
            }
        }

        ++hours;

        LOG_F(INFO, "cron: Check for recurrings");
        check_for_recurrings();

        // We save the cache once per day
        if (hours % 24 == 0) {
            LOG_F(INFO, "cron: Save the caches");
            save_currency_cache();
            save_share_price_cache();
        }

        // Every four hours, we refresh the currency cache
        // Only current day rates are refreshed
        if (hours % 4 == 0) {
            LOG_F(INFO, "cron: Refresh the currency cache");
            budget::refresh_currency_cache();
        }

        // Every hour, we try to prefetch value for new days
        LOG_F(INFO, "cron: Prefetch the share cache");
        budget::prefetch_share_price_cache();
    }

    LOG_F(INFO, "cron: Cron thread has exited");
}

void load() {
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

} // end of anonymous namespace

int main(int argc, char** argv) {
    std::locale const global_locale("");
    std::locale::global(global_locale);

    // Initialize logging for the server
    loguru::g_stderr_verbosity = loguru::Verbosity_INFO;
    loguru::init(argc, argv);

    if (!load_config()) {
        LOG_F(ERROR, "Could not config");
        return 1;
    }

    set_server_running();

    // Restore the caches
    load_currency_cache();
    load_share_price_cache();

    auto old_data_version = to_number<size_t>(internal_config_value("data_version"));

    LOG_F(INFO, "Detected database version {}", old_data_version);

    try {
        if (!budget::migrate_database(old_data_version)) {
            return 1;
        }
    } catch (const budget_exception& e) {
        LOG_F(ERROR, "budget_exception occured in migrate: {}", e.message());
        return 1;
    } catch (const date_exception& e) {
        LOG_F(ERROR, "date_exception occured in migrate: {}", e.message());
        return 1;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "std::exception occured in migrate: {}", e.what());
        return 1;
    }

    // Load all the data
    try {
        load();
    } catch (const budget_exception& e) {
        LOG_F(ERROR, "budget_exception occured in load: {}", e.message());
        return 1;
    } catch (const date_exception& e) {
        LOG_F(ERROR, "date_exception occured in load: {}", e.message());
        return 1;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "std::exception occured in load: {}", e.what());
        return 1;
    }

    std::atomic<bool> success = false;

    {
        std::jthread cron_thread([]() { start_cron_loop(); });

        {
            std::jthread server_thread([&success]() { success = start_server(); });
        }

        if (!success) {
            cron = false;
        }
    }

    // Save the caches
    save_currency_cache();
    save_share_price_cache();

    save_config();

    return 0;
}
