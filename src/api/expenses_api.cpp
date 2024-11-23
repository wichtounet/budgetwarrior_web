//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <ranges>
#include <set>

#include "accounts.hpp"
#include "api/server_api.hpp"
#include "api/expenses_api.hpp"

#include "expenses.hpp"
#include "guid.hpp"
#include "http.hpp"
#include "data.hpp"
#include "views.hpp"
#include "data_cache.hpp"

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

void budget::import_expenses_api(const httplib::Request& req, httplib::Response& res) {
    if (!parameters_present(req, {"n_expenses"})) {
        return api_error(req, res, "Invalid parameters");
    }

    const auto n_expenses = budget::to_number<size_t>(req.get_param_value("n_expenses"));

    size_t imported = 0;
    for (size_t n = 0; n < n_expenses; ++n) {
        auto included_param = std::format("expense_{}_include", n);
        auto id_param       = std::format("expense_{}_id", n);
        auto amount_param   = std::format("expense_{}_amount", n);
        auto name_param     = std::format("expense_{}_name", n);
        auto account_param  = std::format("expense_{}_account", n);

        if (!req.has_param(id_param)) {
            return api_error(req, res, "Invalid parameters in the form");
        }

        auto id = budget::to_number<size_t>(req.get_param_value(id_param));
        if (!budget::expense_exists(id)) {
            return api_error(req, res, "Invalid expense in the form");
        }

        auto expense = budget::expense_get(id);

        if (!expense.temporary) {
            return api_error(req, res, "Invalid expense in the form");
        }

        if (!req.has_param(amount_param) || !req.has_param(name_param) || !req.has_param(account_param)) {
            return api_error(req, res, "Invalid parameters in the form");
        }

        if (!req.has_param(included_param)) {
            budget::expense_delete(id);
            continue;
        }

        auto name    = req.get_param_value(name_param);
        auto amount  = budget::money_from_string(req.get_param_value(amount_param));
        auto account = budget::to_number<size_t>(req.get_param_value(account_param));

        if (!account_exists(account)) {
            return api_error(req, res, "Invalid account in the form");
        }

        expense.name      = name;
        expense.amount    = amount;
        expense.account   = account;
        expense.temporary = false;

        edit_expense(expense);

        ++imported;
    }

    api_success(req, res, std::format("{} expenses have been handled ({} imported)", n_expenses, imported));
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

    if (!std::ranges::contains(columns, "Date") || !std::ranges::contains(columns, "Amount")|| !std::ranges::contains(columns, "Description")) {
        return api_error(req, res, "Invalid file, missing columns");
    }

    size_t date_index = std::distance(columns.begin(), std::ranges::find(columns, "Date"));
    size_t amount_index = std::distance(columns.begin(), std::ranges::find(columns, "Amount"));
    size_t desc_index = std::distance(columns.begin(), std::ranges::find(columns, "Description"));

    size_t added   = 0;
    size_t ignored = 0;

    data_cache cache;

    for (const auto & value : values) {
        // Skip uncomplete lines
        if (value.size() != columns.size()) {
            continue;
        }

        auto date_value = clean_string(value[date_index]);
        auto amount_value = clean_string(value[amount_index]);
        auto desc_value = clean_string(value[desc_index]);

        // Only handle expenses for now
        if (amount_value.front() == '-') {
            amount_value = amount_value.substr(1);
        } else {
            continue;
        }

        auto date = budget::date_from_string(date_value);
        auto amount = budget::money_from_string(amount_value);

        if (cache.expenses() | persistent | filter_by_amount(amount) | filter_by_date(date) | filter_by_original_name(desc_value)) {
            ++ ignored;
            continue;
        }

        expense expense;
        expense.guid    = budget::generate_guid();
        expense.date    = date;

        // TODO Do Beter with translation memory
        if (has_default_account()) {
            expense.account = default_account().id;
        } else {
            expense.account = cache.accounts().front().id;
        }

        expense.name    = desc_value; // TODO Do better using translation memory
        expense.amount  = amount;
        expense.original_name = desc_value;
        expense.temporary = true;

        add_expense(std::move(expense));
        ++added;
    }

    api_success(req, res, std::format("{} expenses have been temporarily imported", added));
}
