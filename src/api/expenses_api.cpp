//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <ranges>
#include <set>

#include "api/server_api.hpp"
#include "api/expenses_api.hpp"

#include "expenses.hpp"
#include "guid.hpp"
#include "http.hpp"
#include "data.hpp"
#include "views.hpp"

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

    for (auto& expense : all_expenses() | persistent) {
        data_writer writer;
        expense.save(writer);
        ss << writer.to_string() << std::endl;
    }

    api_success_content(req, res, ss.str());
}

namespace {

std::string_view clean_string(std::string_view v) {
    if (v.size() < 3)  {
        return v;
    }

    if (v.front() == '\"' && v.back() == '\"') {
        return v.substr(1, v.size() - 2);
    }

    return v;
}

} // namespace

void budget::import_neon_expenses_api(const httplib::Request& req, httplib::Response& res) {
    const auto & file = req.get_file_value("file");
    const auto & file_content = file.content;

    if (!file_content.length()) {
        return api_error(req, res, "Invalid parameters");
    }

    std::vector<std::string_view> columns;
    std::vector<std::vector<std::string_view>> values;

    for (auto line : std::views::split(file_content, '\n')) {
        if (columns.empty()) {
            for (const auto & column : std::views::split(line, ';')) {
                columns.emplace_back(clean_string(std::string_view(column)));
            }

            continue;
        }

        if (std::string_view(line).empty()) {
            continue;
        }

        auto & v = values.emplace_back();
        for (const auto & column : std::views::split(line, ';')) {
            v.emplace_back(clean_string(std::string_view(column)));
        }
    }

    if (columns.empty()) {
        return api_error(req, res, "Invalid file, missing columns");
    }

    if (values.empty()) {
        return api_error(req, res, "Invalid file, missing values");
    }

    api_success(req, res, std::format("Everything has been imported: {} expenses found", values.size()));
}
