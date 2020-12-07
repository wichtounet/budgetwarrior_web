//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "data_cache.hpp"

#include "pages/html_writer.hpp"
#include "pages/asset_values_pages.hpp"
#include "http.hpp"

using namespace budget;

void budget::list_asset_values_page(html_writer& w) {
    budget::list_asset_values(w);

    make_tables_sortable(w);
}

void budget::add_asset_values_page(html_writer& w) {
    w << title_begin << "New asset value" << title_end;

    form_begin(w, "/api/asset_values/add/", "/asset_values/add/");

    add_value_asset_picker(w);
    add_amount_picker(w);
    add_date_picker(w, budget::to_string(budget::local_day()));

    form_end(w);
}

void budget::edit_asset_values_page(html_writer& w, const httplib::Request& req) {
    if (!validate_parameters(w, req, {"input_id", "back_page"})){
        return;
    }

    auto input_id = req.get_param_value("input_id");

    if (!asset_value_exists(budget::to_number<size_t>(input_id))) {
        display_error_message(w, "The asset value " + input_id + " does not exist");
    } else {
        auto back_page = req.get_param_value("back_page");

        w << title_begin << "Edit asset " << input_id << title_end;

        form_begin_edit(w, "/api/asset_values/edit/", back_page, input_id);

        auto asset_value = get_asset_value(budget::to_number<size_t>(input_id));

        add_value_asset_picker(w, budget::to_string(asset_value.asset_id));
        add_amount_picker(w, budget::money_to_string(asset_value.amount));
        add_date_picker(w, budget::to_string(asset_value.set_date));

        form_end(w);
    }
}

void budget::full_batch_asset_values_page(html_writer& w) {
    w << title_begin << "Batch update asset values" << title_end;

    form_begin(w, "/api/asset_values/batch/", "/asset_values/batch/full/");

    add_date_picker(w, budget::to_string(budget::local_day()), true);

    auto assets = w.cache.user_assets();
    std::sort(assets.begin(), assets.end(), [](auto& lhs, auto & rhs) {
        return lhs.name <= rhs.name;
    });

    for (auto& asset : assets) {
        if (!asset.share_based) {
            budget::money amount = get_asset_value(asset, w.cache);

            add_money_picker(w, asset.name, "input_amount_" + budget::to_string(asset.id), budget::money_to_string(amount), true, true, asset.currency);
        }
    }

    form_end(w);
}

void budget::current_batch_asset_values_page(html_writer& w) {
    w << title_begin << "Batch update asset values" << title_end;

    form_begin(w, "/api/asset_values/batch/", "/asset_values/batch/current/");

    add_date_picker(w, budget::to_string(budget::local_day()), true);

    auto assets = w.cache.user_assets();
    std::sort(assets.begin(), assets.end(), [](auto& lhs, auto & rhs) {
        return lhs.name <= rhs.name;
    });

    for (auto& asset : assets) {
        if (!asset.share_based) {
            budget::money amount = get_asset_value(asset, w.cache);

            if (amount) {
                add_money_picker(w, asset.name, "input_amount_" + budget::to_string(asset.id), budget::money_to_string(amount), true, true, asset.currency);
            }
        }
    }

    form_end(w);
}

void budget::add_asset_values_liability_page(html_writer& w) {
    if (no_liabilities()) {
        w << title_begin << "No liabilities" << title_end;
    } else {
        w << title_begin << "New liability asset value" << title_end;

        form_begin(w, "/api/asset_values/add/", "/asset_values/add/liability/");

        add_liability_picker(w);
        add_amount_picker(w);
        add_date_picker(w, budget::to_string(budget::local_day()));

        form_end(w);
    }
}
