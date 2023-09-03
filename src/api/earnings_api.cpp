//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/earnings_api.hpp"

#include "earnings.hpp"
#include "guid.hpp"
#include "http.hpp"
#include "data.hpp"

using namespace budget;

void budget::add_earnings_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_name", "input_date", "input_amount", "input_account"})) {
        return api_error(req, res, "Invalid parameters");
    }

    earning earning;
    earning.guid    = budget::generate_guid();
    earning.date    = budget::date_from_string(req.get_param_value("input_date"));
    earning.account = budget::to_number<size_t>(req.get_param_value("input_account"));
    earning.name    = req.get_param_value("input_name");
    earning.amount  = budget::money_from_string(req.get_param_value("input_amount"));

    auto id = add_earning(std::move(earning));

    api_success(req, res, "Earning " + to_string(id) + " has been created", to_string(id));
}

void budget::edit_earnings_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id", "input_name", "input_date", "input_amount", "input_account"})) {
        return api_error(req, res, "Invalid parameters");
    }

    auto id = req.get_param_value("input_id");

    if (!budget::earning_exists(budget::to_number<size_t>(id))) {
        return api_error(req, res, "Earning " + id + " does not exist");
    }

    earning earning = earning_get(budget::to_number<size_t>(id));
    earning.date    = budget::date_from_string(req.get_param_value("input_date"));
    earning.account = budget::to_number<size_t>(req.get_param_value("input_account"));
    earning.name    = req.get_param_value("input_name");
    earning.amount  = budget::money_from_string(req.get_param_value("input_amount"));

    edit_earning(earning);

    api_success(req, res, "Earning " + to_string(earning.id) + " has been modified");
}

void budget::delete_earnings_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id"})) {
        return api_error(req, res, "Invalid parameters");
    }

    auto id = req.get_param_value("input_id");

    if (!budget::earning_exists(budget::to_number<size_t>(id))) {
        return api_error(req, res, "The earning " + id + " does not exit");
    }

    budget::earning_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Earning " + id + " has been deleted");
}

void budget::list_earnings_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for (auto& earning : all_earnings()) {
        data_writer writer;
        earning.save(writer);
        ss << writer.to_string() << std::endl;
    }

    api_success_content(req, res, ss.str());
}
