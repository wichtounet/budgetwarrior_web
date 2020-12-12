//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>

#include "api/server_api.hpp"
#include "api/user_api.hpp"

#include "http.hpp"
#include "config.hpp"

using namespace budget;

namespace {

bool yes_or_no(const std::string& value) {
    return value == "yes" || value == "no";
}

} // end of anonymous namespace

void budget::user_config_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req,
                            {"input_enable_fortune",
                             "input_enable_debts",
                             "input_default_account",
                             "input_taxes_account",
                             "input_sh_account",
                             "input_sh_prefix",
                             "input_user",
                             "input_password"})) {
        api_error(req, res, "Invalid parameters");
        return;
    }

    if (!yes_or_no(req.get_param_value("input_enable_fortune")) || !yes_or_no(req.get_param_value("input_enable_debts"))) {
        api_error(req, res, "Invalid parameter value");
        return;
    }

    auto disable_fortune = req.get_param_value("input_enable_fortune") == "no";
    internal_config_set("disable_fortune", disable_fortune ? "true" : "false");

    auto disable_debts = req.get_param_value("input_enable_debts") == "no";
    internal_config_set("disable_debts", disable_debts ? "true" : "false");

    internal_config_set("default_account", req.get_param_value("input_default_account"));
    internal_config_set("taxes_account", req.get_param_value("input_taxes_account"));

    internal_config_set("side_category", req.get_param_value("input_sh_account"));
    internal_config_set("side_prefix", req.get_param_value("input_sh_prefix"));

    internal_config_set("web_user", req.get_param_value("input_user"));
    internal_config_set("web_password", req.get_param_value("input_password"));

    budget::save_config();

    api_success(req, res, "Configuration has been updated");
}
