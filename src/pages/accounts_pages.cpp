//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "accounts.hpp"

#include "pages/html_writer.hpp"
#include "pages/accounts_pages.hpp"
#include "http.hpp"

using namespace budget;

namespace {

void add_hide_if_empty_picker(budget::writer& w, bool hide_if_empty) {
    add_yes_no_picker(w, "Hide if empty?", "input_hide_if_empty", hide_if_empty);
}

}

void budget::accounts_page(html_writer& w) {
    budget::show_accounts(w);

    make_tables_sortable(w);
}

void budget::all_accounts_page(html_writer& w) {
    budget::show_all_accounts(w);

    make_tables_sortable(w);
}

void budget::add_accounts_page(html_writer& w) {
    w << title_begin << "New account" << title_end;

    form_begin(w, "/api/accounts/add/", "/accounts/add/");

    add_name_picker(w);
    add_amount_picker(w);
    add_hide_if_empty_picker(w, false);

    form_end(w);
}

void budget::edit_accounts_page(html_writer& w, const httplib::Request& req) {
    if (!req.has_param("input_id") || !req.has_param("back_page")) {
        return display_error_message(w, "Invalid parameter for the request");
    }

    auto input_id = req.get_param_value("input_id");

    if (!account_exists(budget::to_number<size_t>(input_id))) {
        return display_error_message(w, "The account {} does not exist", input_id);
    }

    auto back_page = req.get_param_value("back_page");

    w << title_begin << "Edit account " << input_id << title_end;

    form_begin_edit(w, "/api/accounts/edit/", back_page, input_id);

    auto account = get_account(budget::to_number<size_t>(input_id));

    add_name_picker(w, account.name);
    add_amount_picker(w, budget::money_to_string(account.amount));
    add_hide_if_empty_picker(w, account.hide_if_empty);

    form_end(w);
}

void budget::archive_accounts_month_page(html_writer& w) {
    w << title_begin << "Archive accounts from the beginning of the month" << title_end;

    form_begin(w, "/api/accounts/archive/month/", "/accounts/");

    w << "<p>This will create new accounts that will be used starting from the beginning of the current month. Are you sure you want to proceed ? </p>";

    form_end(w, "Confirm");
}

void budget::archive_accounts_year_page(html_writer& w) {
    w << title_begin << "Archive accounts from the beginning of the year" << title_end;

    form_begin(w, "/api/accounts/archive/year/", "/accounts/");

    w << "<p>This will create new accounts that will be used starting from the beginning of the current year. Are you sure you want to proceed ? </p>";

    form_end(w, "Confirm");
}
