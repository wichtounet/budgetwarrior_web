//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "liabilities.hpp"

#include "pages/html_writer.hpp"
#include "pages/liabilities_pages.hpp"
#include "http.hpp"

using namespace budget;

namespace {

void add_currency_picker(budget::writer& w, std::string_view default_value = "") {
    add_text_picker(w, "Currency", "input_currency", default_value);
}

} // namespace

void budget::list_liabilities_page(html_writer& w) {
    budget::show_liabilities(w);

    make_tables_sortable(w);
}

void budget::add_liabilities_page(html_writer& w) {
    w << title_begin << "New liability" << title_end;

    form_begin(w, "/api/liabilities/add/", "/liabilities/add/");

    add_name_picker(w);

    for (const auto& clas : all_asset_classes()) {
        add_money_picker(w, clas.name + " (%)", "input_class_" + to_string(clas.id), "");
    }

    add_currency_picker(w);

    form_end(w);
}

void budget::edit_liabilities_page(html_writer& w, const httplib::Request& req) {
    if (!validate_parameters(w, req, {"input_id", "back_page"})) {
        return;
    }

    auto input_id = req.get_param_value("input_id");
    auto id       = budget::to_number<size_t>(input_id);

    if (!liability_exists(id)) {
        return display_error_message(w, "The liability {} does not exist", input_id);
    }

    auto back_page = req.get_param_value("back_page");

    w << title_begin << "Edit liability " << input_id << title_end;

    form_begin_edit(w, "/api/liabilities/edit/", back_page, input_id);

    auto liability = get_liability(id);

    add_name_picker(w, liability.name);

    for (const auto& clas : all_asset_classes()) {
        add_money_picker(w, clas.name + " (%)", "input_class_" + to_string(clas.id), budget::money_to_string(get_asset_class_allocation(liability, clas)));
    }

    add_currency_picker(w, liability.currency);

    form_end(w);
}
