//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/expenses_api.hpp"

#include "expenses.hpp"
#include "guid.hpp"
#include "http.hpp"
#include "data.hpp"

using namespace budget;

void budget::add_expenses_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_name", "input_date", "input_amount", "input_account"})) {
        return api_error(req, res, "Invalid parameters");
    }

    expense expense;
    expense.guid    = budget::generate_guid();
    expense.date    = budget::date_from_string(req.get_param_value("input_date"));
    expense.account = budget::to_number<size_t>(req.get_param_value("input_account"));
    expense.name    = req.get_param_value("input_name");
    expense.amount  = budget::money_from_string(req.get_param_value("input_amount"));

    auto id = add_expense(std::move(expense));

    api_success(req, res, "Expense " + to_string(id) + " has been created", to_string(id));
}

void budget::edit_expenses_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id", "input_name", "input_date", "input_amount", "input_account"})) {
        return api_error(req, res, "Invalid parameters");
    }

    auto id = req.get_param_value("input_id");

    if (!budget::expense_exists(budget::to_number<size_t>(id))) {
        return api_error(req, res, "Expense " + id + " does not exist");
    }

    expense expense = expense_get(budget::to_number<size_t>(id));
    expense.date    = budget::date_from_string(req.get_param_value("input_date"));
    expense.account = budget::to_number<size_t>(req.get_param_value("input_account"));
    expense.name    = req.get_param_value("input_name");
    expense.amount  = budget::money_from_string(req.get_param_value("input_amount"));

    edit_expense(expense);

    api_success(req, res, "Expense " + to_string(expense.id) + " has been modified");
}

void budget::delete_expenses_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"input_id"})) {
        return api_error(req, res, "Invalid parameters");
    }

    auto id = req.get_param_value("input_id");

    if (!budget::expense_exists(budget::to_number<size_t>(id))) {
        return api_error(req, res, "The expense " + id + " does not exit");
    }

    budget::expense_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Expense " + id + " has been deleted");
}

void budget::list_expenses_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for (auto& expense : all_expenses()) {
        data_writer writer;
        expense.save(writer);
        ss << writer.to_string() << std::endl;
    }

    api_success_content(req, res, ss.str());
}
