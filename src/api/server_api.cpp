//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/earnings_api.hpp"
#include "api/expenses_api.hpp"
#include "api/accounts_api.hpp"
#include "api/incomes_api.hpp"
#include "api/objectives_api.hpp"
#include "api/recurrings_api.hpp"
#include "api/debts_api.hpp"
#include "api/wishes_api.hpp"
#include "api/fortunes_api.hpp"
#include "api/assets_api.hpp"
#include "api/user_api.hpp"

#include "pages/server_pages.hpp"

#include "config.hpp"
#include "version.hpp"
#include "writer.hpp"
#include "http.hpp"
#include "logging.hpp"

using namespace budget;

namespace {

void server_up_api(const httplib::Request& req, httplib::Response& res) {
    api_success_content(req, res, "yes");
}

void server_version_api(const httplib::Request& req, httplib::Response& res) {
    api_success_content(req, res, get_version_short());
}

void server_version_support_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"version"})) {
        return api_error(req, res, "Invalid parameters");
    }

    auto client_version = req.get_param_value("version");

    if (client_version == "1.1" || client_version == "1.1.0" || client_version == "1.1.1") {
        api_success_content(req, res, "yes");
    } else {
        api_success_content(req, res, "no");
    }
}

void retirement_configure_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_wrate", "input_roi"})) {
        return api_error(req, res, "Invalid parameters");
    }

    // Save the configuration
    internal_config_set("withdrawal_rate", req.get_param_value("input_wrate"));
    internal_config_set("expected_roi", req.get_param_value("input_roi"));

    save_config();

    api_success(req, res, "Retirement configuration was saved");
}

auto api_wrapper(void (*api_function)(const httplib::Request&, httplib::Response&)) {
    return [api_function](const httplib::Request& req, httplib::Response& res) {
        try {
            if (!api_start(req, res)) {
                return;
            }

            api_function(req, res);
        } catch (const budget_exception& e) {
            api_error(req, res, std::format("Exception occurred: ", e.message()));
            LOG_F(ERROR, "budget_exception occured in render({}): {}", req.path, e.message());
        } catch (const date_exception& e) {
            api_error(req, res, std::format("Exception occurred: ",  e.message()));
            LOG_F(ERROR, "date_exception occured in render({}): {}", req.path, e.message());
        } catch (const std::exception & e) {
            api_error(req, res, std::format("Exception occurred: {}", e.what()));
            LOG_F(ERROR, "std_exception occured in render({}): {}", req.path, e.what());
        }
    };
}

std::string encode_url(std::string_view s) {
    std::string result;

    for (auto i = 0; s[i]; i++) {
        switch (s[i]) {
        case ' ':
            result += "%20";
            break;
        case '+':
            result += "%2B";
            break;
        case '\r':
            result += "%0D";
            break;
        case '\n':
            result += "%0A";
            break;
        case '\'':
            result += "%27";
            break;
        case ',':
            result += "%2C";
            break;
        case ':':
            result += "%3A";
            break;
        case ';':
            result += "%3B";
            break;
        default:
            auto c = static_cast<uint8_t>(s[i]);
            if (c >= 0x80) {
                result += '%';
                char         hex[4];
                const size_t len = snprintf(hex, sizeof(hex) - 1, "%02X", c);
                assert(len == 2);
                result.append(hex, len);
            } else {
                result += s[i];
            }
            break;
        }
    }

    return result;
}

} // end of anonymous namespace

void budget::load_api(httplib::Server& server) {
    server.Get("/api/server/up/", api_wrapper(&server_up_api));
    server.Get("/api/server/version/", api_wrapper(&server_version_api));
    server.Post("/api/server/version/support/", api_wrapper(&server_version_support_api));

    server.Post("/api/accounts/add/", api_wrapper(&add_accounts_api));
    server.Post("/api/accounts/edit/", api_wrapper(&edit_accounts_api));
    server.Get("/api/accounts/delete/", api_wrapper(&delete_accounts_api));
    server.Post("/api/accounts/archive/month/", api_wrapper(&archive_accounts_month_api));
    server.Post("/api/accounts/archive/year/", api_wrapper(&archive_accounts_year_api));
    server.Get("/api/accounts/list/", api_wrapper(&list_accounts_api));

    server.Post("/api/incomes/add/", api_wrapper(&add_incomes_api));
    server.Post("/api/incomes/edit/", api_wrapper(&edit_incomes_api));
    server.Get("/api/incomes/delete/", api_wrapper(&delete_incomes_api));
    server.Get("/api/incomes/list/", api_wrapper(&list_incomes_api));

    server.Post("/api/expenses/add/", api_wrapper(&add_expenses_api));
    server.Post("/api/expenses/edit/", api_wrapper(&edit_expenses_api));
    server.Get("/api/expenses/delete/", api_wrapper(&delete_expenses_api));
    server.Get("/api/expenses/list/", api_wrapper(&list_expenses_api));
    server.Post("/api/expenses/import/", api_wrapper(&import_expenses_api));
    server.Post("/api/expenses/import/neon/", api_wrapper(&import_neon_expenses_api));

    server.Post("/api/earnings/add/", api_wrapper(&add_earnings_api));
    server.Post("/api/earnings/edit/", api_wrapper(&edit_earnings_api));
    server.Get("/api/earnings/delete/", api_wrapper(&delete_earnings_api));
    server.Get("/api/earnings/list/", api_wrapper(&list_earnings_api));

    server.Post("/api/recurrings/add/", api_wrapper(&add_recurrings_api));
    server.Post("/api/recurrings/edit/", api_wrapper(&edit_recurrings_api));
    server.Get("/api/recurrings/delete/", api_wrapper(&delete_recurrings_api));
    server.Get("/api/recurrings/list/", api_wrapper(&list_recurrings_api));

    server.Post("/api/debts/add/", api_wrapper(&add_debts_api));
    server.Post("/api/debts/edit/", api_wrapper(&edit_debts_api));
    server.Get("/api/debts/delete/", api_wrapper(&delete_debts_api));
    server.Get("/api/debts/list/", api_wrapper(&list_debts_api));

    server.Post("/api/fortunes/add/", api_wrapper(&add_fortunes_api));
    server.Post("/api/fortunes/edit/", api_wrapper(&edit_fortunes_api));
    server.Get("/api/fortunes/delete/", api_wrapper(&delete_fortunes_api));
    server.Get("/api/fortunes/list/", api_wrapper(&list_fortunes_api));

    server.Post("/api/wishes/add/", api_wrapper(&add_wishes_api));
    server.Post("/api/wishes/edit/", api_wrapper(&edit_wishes_api));
    server.Get("/api/wishes/delete/", api_wrapper(&delete_wishes_api));
    server.Get("/api/wishes/list/", api_wrapper(&list_wishes_api));

    server.Post("/api/assets/add/", api_wrapper(&add_assets_api));
    server.Post("/api/assets/edit/", api_wrapper(&edit_assets_api));
    server.Get("/api/assets/delete/", api_wrapper(&delete_assets_api));
    server.Get("/api/assets/list/", api_wrapper(&list_assets_api));

    server.Post("/api/asset_values/add/", api_wrapper(&add_asset_values_api));
    server.Post("/api/asset_values/edit/", api_wrapper(&edit_asset_values_api));
    server.Post("/api/asset_values/batch/", api_wrapper(&batch_asset_values_api));
    server.Get("/api/asset_values/delete/", api_wrapper(&delete_asset_values_api));
    server.Get("/api/asset_values/list/", api_wrapper(&list_asset_values_api));

    server.Post("/api/asset_shares/add/", api_wrapper(&add_asset_shares_api));
    server.Post("/api/asset_shares/edit/", api_wrapper(&edit_asset_shares_api));
    server.Get("/api/asset_shares/delete/", api_wrapper(&delete_asset_shares_api));
    server.Get("/api/asset_shares/list/", api_wrapper(&list_asset_shares_api));

    server.Post("/api/asset_classes/add/", api_wrapper(&add_asset_classes_api));
    server.Post("/api/asset_classes/edit/", api_wrapper(&edit_asset_classes_api));
    server.Get("/api/asset_classes/delete/", api_wrapper(&delete_asset_classes_api));
    server.Get("/api/asset_classes/list/", api_wrapper(&list_asset_classes_api));

    server.Post("/api/liabilities/add/", api_wrapper(&add_liabilities_api));
    server.Post("/api/liabilities/edit/", api_wrapper(&edit_liabilities_api));
    server.Get("/api/liabilities/delete/", api_wrapper(&delete_liabilities_api));
    server.Get("/api/liabilities/list/", api_wrapper(&list_liabilities_api));

    server.Post("/api/retirement/configure/", api_wrapper(&retirement_configure_api));

    server.Post("/api/objectives/add/", api_wrapper(&add_objectives_api));
    server.Post("/api/objectives/edit/", api_wrapper(&edit_objectives_api));
    server.Get("/api/objectives/delete/", api_wrapper(&delete_objectives_api));
    server.Get("/api/objectives/list/", api_wrapper(&list_objectives_api));

    server.Post("/api/user/config/", api_wrapper(&user_config_api));
}

bool budget::api_start(const httplib::Request& req, httplib::Response& res) {
    return authenticate(req, res);
}

void budget::api_error(const httplib::Request& req, httplib::Response& res, std::string_view message) {
    if (req.has_param("server")) {
        auto back_page = html_base64_decode(req.get_param_value("back_page"));

        std::string url;
        if (back_page.find('?') == std::string::npos) {
            url = std::format("{}?error=true&message={}", back_page, encode_url(message));
        } else {
            url = std::format("{}&error=true&message={}", back_page, encode_url(message));
        }

        res.set_redirect(url);
    } else {
        res.set_content(std::format("Error: {}", message), "text/plain");
    }
}

void budget::api_success(const httplib::Request& req, httplib::Response& res, std::string_view message) {
    if (req.has_param("server")) {
        auto back_page = html_base64_decode(req.get_param_value("back_page"));

        std::string url;
        if (back_page.find('?') == std::string::npos) {
            url = std::format("{}?success=true&message={}", back_page, encode_url(message));
        } else {
            url = std::format("{}&success=true&message={}", back_page, encode_url(message));
        }

        res.set_redirect(url);
    } else {
        res.set_content(std::format("Success: {}", message), "text/plain");
    }
}

void budget::api_success(const httplib::Request& req, httplib::Response& res, std::string_view message, const std::string& content) {
    if (req.has_param("server")) {
        auto back_page = html_base64_decode(req.get_param_value("back_page"));

        std::string url;
        if (back_page.find('?') == std::string::npos) {
            url = std::format("{}?success=true&message={}", back_page, encode_url(message));
        } else {
            url = std::format("{}&success=true&message={}", back_page, encode_url(message));
        }

        res.set_redirect(url);
    } else {
        res.set_content(content, "text/plain");
    }
}

void budget::api_success_content(const httplib::Request& /*req*/, httplib::Response& res, const std::string& content) {
    res.set_content(content, "text/plain");
}

bool budget::parameters_present(const httplib::Request& req, const std::vector<const char*>& parameters) {
    return std::ranges::all_of(parameters, [&req](const auto& param) { return req.has_param(param); });
}
