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

    budget::date today = budget::local_day();

    std::string def = has_default_account() ? default_account().name : "";
    add_account_picker_by_name(w, today, "Default account", def, "input_default_account");

    form_end(w);
}
