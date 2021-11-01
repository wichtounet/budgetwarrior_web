//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "assets.hpp"

#include "pages/html_writer.hpp"
#include "pages/asset_classes_pages.hpp"
#include "http.hpp"

using namespace budget;

namespace {

void add_fi_picker(budget::writer& w, bool active) {
    add_yes_no_picker(w, "FI?", "input_fi", active);
}

} // namespace

void budget::list_asset_classes_page(html_writer & w) {
    budget::show_asset_classes(w);

    make_tables_sortable(w);
}

void budget::add_asset_classes_page(html_writer & w) {
    w << title_begin << "New asset class" << title_end;

    form_begin(w, "/api/asset_classes/add/", "/asset_classes/add/");

    add_name_picker(w);
    add_fi_picker(w, true);

    form_end(w);
}

void budget::edit_asset_classes_page(html_writer & w, const httplib::Request& req) {
    if (!validate_parameters(w, req, {"input_id", "back_page"})){
        return;
    }

    auto input_id = req.get_param_value("input_id");
    auto id = budget::to_number<size_t>(input_id);

    if (!asset_class_exists(id)) {
        display_error_message(w, "The asset class " + input_id + " does not exist");
    } else {
        auto back_page = req.get_param_value("back_page");

        w << title_begin << "Edit asset class " << input_id << title_end;

        form_begin_edit(w, "/api/asset_classes/edit/", back_page, input_id);

        auto asset_class = get_asset_class(id);

        add_name_picker(w, asset_class.name);
        add_fi_picker(w, asset_class.fi);

        form_end(w);
    }
}
