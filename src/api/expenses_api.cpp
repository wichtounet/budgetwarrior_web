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

std::pair<std::vector<std::string_view>, std::vector<std::vector<std::string_view>>> parse_csv(std::string_view file_content, char sep) {
    std::vector<std::string_view>              columns;
    std::vector<std::vector<std::string_view>> values;

    for (auto line : std::views::split(file_content, '\n')) {
        // Skip empty lines
        if (std::string_view(line).empty()) {
            continue;
        }

        if (columns.empty()) {
            for (const auto& column : std::views::split(line, sep)) {
                columns.emplace_back(clean_string(std::string_view(column)));
            }

            continue;
        }

        auto & v = values.emplace_back();

        std::string_view column_acc;
        bool acc = false;

        for (const auto& column : std::views::split(line, sep)) {
            std::string_view column_sv(column);

            // Very simple algorithm to handle separators in between quotes
            if (column_sv.front() == '\"' && column_sv.back() != '\"') {
                column_acc = column_sv;
                acc = true;
            } else if (acc && column_sv.back() != '\"') {
                column_acc = std::string_view(column_acc.data(), column_acc.size() + column_sv.size() + 1);
            } else if (acc && column_sv.back() == '\"') {
                column_acc = std::string_view(column_acc.data(), column_acc.size() + column_sv.size() + 1);
                acc = false;
                v.emplace_back(clean_string(column_acc));
            } else {
                v.emplace_back(clean_string(column_sv));
            }
        }
    }

    return {columns, values};
}

void import_expense(data_cache & cache, std::string_view desc_value, budget::money amount, budget::date date, size_t & ignored, size_t & added) {
    if (cache.expenses() | persistent | filter_by_amount(amount) | filter_by_date(date) | filter_by_original_name(desc_value)) {
        ++ ignored;
        return;
    }

    expense expense;
    expense.guid    = budget::generate_guid();
    expense.date    = date;

    // By default, we use either the default account or the first account
    if (has_default_account()) {
        expense.account = default_account().id;
    } else {
        expense.account = cache.accounts().front().id;
    }

    // By default, we guess the name as the description value
    expense.name = desc_value;

    // Then, we use the translation memory to do better for names and accounts

    auto same_original_name        = cache.expenses() | filter_by_original_name(desc_value);
    auto same_original_name_amount = cache.expenses() | filter_by_original_name(desc_value) | filter_by_amount(amount);

    if (same_original_name) {
        std::string guessed_name    = std::begin(same_original_name)->name;
        size_t      guessed_account = std::begin(same_original_name)->account;

        if (std::ranges::all_of(same_original_name, [&guessed_name](const auto& expense) { return expense.name == guessed_name; })) {
            // If they were always translate the same way, we can reuse the name directly
            expense.name    = guessed_name;
            expense.account = get_account(get_account_name(guessed_account), date.year(), date.month()).id;
        } else if (same_original_name_amount) {
            // Otherwise, we also filter by amount

            guessed_name    = std::begin(same_original_name_amount)->name;
            guessed_account = std::begin(same_original_name)->account;

            if (std::ranges::all_of(same_original_name_amount, [&guessed_name](const auto& expense) { return expense.name == guessed_name; })) {
                expense.name    = guessed_name;
                expense.account = get_account(get_account_name(guessed_account), date.year(), date.month()).id;
            }
        }

        // Note: We could try to be even smarter and recognize the days in the month of imported expenses
        // Or, we could even use the most recent translation as the source of truth, but this can come later
    }

    expense.amount  = amount;
    expense.original_name = desc_value;
    expense.temporary = true;

    add_expense(std::move(expense));
    ++added;
}

} // namespace

void budget::import_neon_expenses_api(const httplib::Request& req, httplib::Response& res) {
    const auto & file = req.get_file_value("file");
    const auto & file_content = file.content;

    if (!file_content.length()) {
        return api_error(req, res, "Invalid parameters");
    }

    auto [columns, values] = parse_csv(file_content, ';');

    if (columns.empty()) {
        return api_error(req, res, "Invalid file, missing columns");
    }

    if (values.empty()) {
        return api_error(req, res, "Invalid file, missing values");
    }

    if (!range_contains(columns, "Date") || !range_contains(columns, "Amount")|| !range_contains(columns, "Description")) {
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

        const auto date_value = clean_string(value[date_index]);
        const auto desc_value = clean_string(value[desc_index]);

        // Only handle expenses for now
        auto amount_value = clean_string(value[amount_index]);
        if (amount_value.front() == '-') {
            amount_value = amount_value.substr(1);
        } else {
            continue;
        }

        const auto date = budget::date_from_string(date_value);
        const auto amount = budget::money_from_string(amount_value);

        import_expense(cache, desc_value, amount, date, ignored, added);
    }

    api_success(req, res, std::format("{} expenses have been temporarily imported ({} ignored)", added, ignored));
}

// Assume the CSV comes from Zamzar
void budget::import_cembra_expenses_api(const httplib::Request& req, httplib::Response& res) {
    const auto & file = req.get_file_value("file");
    const auto & file_content = file.content;

    if (!file_content.length()) {
        return api_error(req, res, "Invalid parameters");
    }

    auto [columns, values] = parse_csv(file_content, ',');

    if (columns.empty()) {
        return api_error(req, res, "Invalid file, missing columns");
    }

    if (values.empty()) {
        return api_error(req, res, "Invalid file, missing values");
    }

    if (!std::ranges::contains(columns, "Date de trans.") || !std::ranges::contains(columns, "Crédit CHF")|| !std::ranges::contains(columns, "Description")) {
        return api_error(req, res, "Invalid file, missing columns");
    }

    size_t date_index = std::distance(columns.begin(), std::ranges::find(columns, "Date de trans."));
    size_t amount_index = std::distance(columns.begin(), std::ranges::find(columns, "Crédit CHF"));
    size_t desc_index = std::distance(columns.begin(), std::ranges::find(columns, "Description"));

    size_t added   = 0;
    size_t ignored = 0;

    data_cache cache;

    for (const auto & value : values) {
        // Skip uncomplete lines
        if (value.size() != columns.size()) {
            continue;
        }

        const auto desc = clean_string(value[desc_index]);

        const auto date_value = clean_string(value[date_index]);
        const auto date       = budget::dmy_date_from_string(date_value);

        const auto amount_value = clean_string(value[amount_index]);
        const auto amount       = budget::single_money_from_string(amount_value);

        import_expense(cache, desc, amount, date, ignored, added);
    }

    api_success(req, res, std::format("{} expenses have been temporarily imported ({} ignored)", added, ignored));
}
