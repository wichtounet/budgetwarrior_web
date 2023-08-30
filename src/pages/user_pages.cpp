//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "config.hpp"
#include "accounts.hpp"

#include "pages/html_writer.hpp"
#include "pages/user_pages.hpp"
#include "http.hpp"

using namespace budget;

void budget::user_config_page(html_writer& w) {
    w << title_begin << "Configuration" << title_end;

    form_begin(w, "/api/user/config/", "/user/config/");

    add_yes_no_picker(w, "Enable fortune module", "input_enable_fortune", !budget::is_fortune_disabled());
    add_yes_no_picker(w, "Enable debts module", "input_enable_debts", !budget::is_debts_disabled());

    budget::date const today = budget::local_day();

    std::string const def = has_default_account() ? default_account().name : "";
    add_account_picker_by_name(w, today, "Default account", def, "input_default_account");

    std::string const taxes = has_taxes_account() ? taxes_account().name : "";
    add_account_picker_by_name(w, today, "Taxes account", taxes, "input_taxes_account", true);

    std::string const sh_account = user_config_value("side_category", "");
    add_account_picker_by_name(w, today, "Side Hustle Account", sh_account, "input_sh_account", true);

    std::string const sh_prefix = user_config_value("side_prefix", "");
    add_text_picker(w, "Side Hustle Prefix", "input_sh_prefix", sh_prefix, false);

    std::string const fi_expenses = user_config_value("fi_expenses", "");
    add_text_picker(w, "FI Expenses", "input_fi_expenses", fi_expenses);

    add_text_picker(w, "User", "input_user", get_web_user());
    add_password_picker(w, "Password", "input_password", get_web_password());

    form_end(w);
}
