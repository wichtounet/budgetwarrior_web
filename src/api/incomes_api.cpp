//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/incomes_api.hpp"

#include "incomes.hpp"
#include "guid.hpp"
#include "http.hpp"
#include "data.hpp"

using namespace budget;

void budget::add_incomes_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_amount"})) {
        return api_error(req, res, "Invalid parameters");
    }

    auto amount = budget::money_from_string(req.get_param_value("input_amount"));

    auto income = budget::new_income(amount, false);

    api_success(req, res, "Income " + to_string(income.id) + " has been created", to_string(income.id));
}

void budget::edit_incomes_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id", "input_amount"})) {
        return api_error(req, res, "Invalid parameters");
    }

    auto id = req.get_param_value("input_id");

    if (!budget::income_exists(budget::to_number<size_t>(id))) {
        return api_error(req, res, "Income " + id + " does not exist");
    }

    income income = income_get(budget::to_number<size_t>(id));
    income.amount = budget::money_from_string(req.get_param_value("input_amount"));

    edit_income(income);

    api_success(req, res, "Income " + to_string(income.id) + " has been modified");
}

void budget::delete_incomes_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id"})) {
        return api_error(req, res, "Invalid parameters");
    }

    auto id = req.get_param_value("input_id");

    if (!budget::income_exists(budget::to_number<size_t>(id))) {
        return api_error(req, res, "The income " + id + " does not exit");
    }

    budget::income_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Income " + id + " has been deleted");
}

void budget::list_incomes_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for (auto& income : all_incomes()) {
        data_writer writer;
        income.save(writer);
        ss << writer.to_string() << std::endl;
    }

    api_success_content(req, res, ss.str());
}
