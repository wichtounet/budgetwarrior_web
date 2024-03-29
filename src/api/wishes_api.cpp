//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/wishes_api.hpp"

#include "wishes.hpp"
#include "accounts.hpp"
#include "guid.hpp"
#include "http.hpp"
#include "data.hpp"

using namespace budget;

void budget::add_wishes_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_urgency") || !req.has_param("input_importance")) {
        return api_error(req, res, "Invalid parameters");
    }

    wish wish;
    wish.paid        = false;
    wish.paid_amount = 0;
    wish.guid        = budget::generate_guid();
    wish.date        = budget::local_day();
    wish.name        = req.get_param_value("input_name");
    wish.importance  = budget::to_number<int>(req.get_param_value("input_importance"));
    wish.urgency     = budget::to_number<int>(req.get_param_value("input_urgency"));
    wish.amount      = budget::money_from_string(req.get_param_value("input_amount"));

    const auto id = add_wish(std::move(wish));
    api_success(req, res, "wish " + to_string(id) + " has been created", to_string(id));
}

void budget::edit_wishes_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_id") || !req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_urgency")
        || !req.has_param("input_importance") || !req.has_param("input_paid") || !req.has_param("input_paid_amount")) {
        return api_error(req, res, "Invalid parameters");
    }

    auto id = req.get_param_value("input_id");

    if (!budget::wish_exists(budget::to_number<size_t>(id))) {
        return api_error(req, res, "wish " + id + " does not exist");
    }

    const bool paid = req.get_param_value("input_paid") == "yes";

    wish wish       = wish_get(budget::to_number<size_t>(id));
    wish.name       = req.get_param_value("input_name");
    wish.importance = budget::to_number<int>(req.get_param_value("input_importance"));
    wish.urgency    = budget::to_number<int>(req.get_param_value("input_urgency"));
    wish.amount     = budget::money_from_string(req.get_param_value("input_amount"));
    wish.paid       = paid;

    if (paid) {
        wish.paid_amount = budget::money_from_string(req.get_param_value("input_paid_amount"));
    }

    edit_wish(wish);

    api_success(req, res, "wish " + to_string(wish.id) + " has been modified");
}

void budget::delete_wishes_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_id")) {
        return api_error(req, res, "Invalid parameters");
    }

    auto id = req.get_param_value("input_id");

    if (!budget::wish_exists(budget::to_number<size_t>(id))) {
        return api_error(req, res, "The wish " + id + " does not exit");
    }

    budget::wish_delete(budget::to_number<size_t>(id));

    api_success(req, res, "wish " + id + " has been deleted");
}

void budget::list_wishes_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for (auto& wish : all_wishes()) {
        data_writer writer;
        wish.save(writer);
        ss << writer.to_string() << std::endl;
    }

    api_success_content(req, res, ss.str());
}
