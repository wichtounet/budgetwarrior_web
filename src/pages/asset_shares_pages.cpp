//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "assets.hpp"

#include "pages/html_writer.hpp"
#include "pages/asset_shares_pages.hpp"
#include "http.hpp"

using namespace budget;

void budget::list_asset_shares_page(html_writer& w) {
    budget::list_asset_shares(w);

    make_tables_sortable(w);
}

void budget::add_asset_shares_page(html_writer& w) {
    w << title_begin << "New asset share" << title_end;

    form_begin(w, "/api/asset_shares/add/", "/asset_shares/add/");

    add_active_share_asset_picker(w);
    add_integer_picker(w, "shares", "input_shares", true);
    add_money_picker(w, "price", "input_price", "");
    add_date_picker(w, budget::to_string(budget::local_day()));

    form_end(w);
}

void budget::edit_asset_shares_page(html_writer& w, const httplib::Request& req) {
    if (!validate_parameters(w, req, {"input_id", "back_page"})) {
        return;
    }

    auto input_id = req.get_param_value("input_id");
    auto id       = budget::to_number<size_t>(input_id);

    if (!asset_share_exists(id)) {
        return display_error_message(w, "The asset share {} does not exist", input_id);
    }

    auto back_page = req.get_param_value("back_page");

    w << title_begin << "Edit asset share " << input_id << title_end;

    form_begin_edit(w, "/api/asset_shares/edit/", back_page, input_id);

    auto asset_share = get_asset_share(id);

    add_share_asset_picker(w, budget::to_string(asset_share.asset_id));
    add_integer_picker(w, "shares", "input_shares", true, budget::to_string(asset_share.shares));
    add_money_picker(w, "price", "input_price", budget::to_string(asset_share.price));
    add_date_picker(w, budget::to_string(asset_share.date));

    form_end(w);
}
