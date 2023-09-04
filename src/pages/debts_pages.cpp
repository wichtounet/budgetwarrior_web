//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "debts.hpp"

#include "pages/html_writer.hpp"
#include "pages/debts_pages.hpp"
#include "http.hpp"

using namespace budget;

namespace {

void add_direction_picker(budget::writer& w, std::string_view default_value = "") {
    w << R"=====(
            <div class="form-group">
                <label for="input_direction">Direction</label>
                <select class="form-control" id="input_direction" name="input_direction">
    )=====";

    if ("to" == default_value) {
        w << "<option selected value=\"to\">To</option>";
    } else {
        w << "<option value=\"to\">To</option>";
    }

    if ("from" == default_value) {
        w << "<option selected value=\"from\">From</option>";
    } else {
        w << "<option value=\"from\">From</option>";
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

} // end of anonymous namespace

void budget::list_debts_page(html_writer& w) {
    budget::list_debts(w);

    make_tables_sortable(w);
}

void budget::all_debts_page(html_writer& w) {
    budget::display_all_debts(w);

    make_tables_sortable(w);
}

void budget::add_debts_page(html_writer& w) {
    w << title_begin << "New Debt" << title_end;

    form_begin(w, "/api/debts/add/", "/debts/add/");

    add_direction_picker(w);
    add_name_picker(w);
    add_amount_picker(w);
    add_title_picker(w);

    form_end(w);
}

void budget::edit_debts_page(html_writer& w, const httplib::Request& req) {
    if (!validate_parameters(w, req, {"input_id", "back_page"})){
        return;
    }

    auto input_id = req.get_param_value("input_id");

    if (!debt_exists(budget::to_number<size_t>(input_id))) {
        display_error_message(w, "The debt {} does not exist", input_id);
    } else {
        auto back_page = req.get_param_value("back_page");

        w << title_begin << "Edit Debt " << input_id << title_end;

        form_begin_edit(w, "/api/debts/edit/", back_page, input_id);

        auto debt = debt_get(budget::to_number<size_t>(input_id));

        add_direction_picker(w, debt.direction ? "to" : "from");
        add_name_picker(w, debt.name);
        add_amount_picker(w, budget::money_to_string(debt.amount));
        add_title_picker(w, debt.title);
        add_paid_picker(w, debt.state == 1);

        form_end(w);
    }
}
