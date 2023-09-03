//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "api/server_api.hpp"
#include "api/recurrings_api.hpp"

#include "recurring.hpp"
#include "accounts.hpp"
#include "guid.hpp"
#include "http.hpp"
#include "data.hpp"

using namespace budget;

void budget::add_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_account") || !req.has_param("input_recurs")
        || !req.has_param("input_type")) {
        return  api_error(req, res, "Invalid parameters");
    }

    recurring recurring;
    recurring.guid    = generate_guid();
    recurring.account = get_account(budget::to_number<size_t>(req.get_param_value("input_account"))).name;
    recurring.name    = req.get_param_value("input_name");
    recurring.amount  = money_from_string(req.get_param_value("input_amount"));
    recurring.recurs  = req.get_param_value("input_recurs");
    recurring.type    = req.get_param_value("input_type");

    if (recurring.recurs != "monthly" && recurring.recurs != "weekly") {
        api_error(req, res, "Invalid recurring frequency");
        return;
    }

    if (recurring.type != "earning" && recurring.type != "expense") {
        api_error(req, res, "Invalid recurring type");
        return;
    }

    auto id = add_recurring(std::move(recurring));
    api_success(req, res, "Recurring Operation" + to_string(id) + " has been created", to_string(id));
}

void budget::edit_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_id") || !req.has_param("input_name") || !req.has_param("input_amount") || !req.has_param("input_account")
        || !req.has_param("input_recurs")) {
        return api_error(req, res, "Invalid parameters");
    }

    auto id = req.get_param_value("input_id");

    if (!recurring_exists(budget::to_number<size_t>(id))) {
        return api_error(req, res, "recurring " + id + " does not exist");
    }

    auto recurring          = recurring_get(budget::to_number<size_t>(id));
    auto previous_recurring = recurring; // Temporary Copy

    recurring.account = get_account(budget::to_number<size_t>(req.get_param_value("input_account"))).name;
    recurring.name    = req.get_param_value("input_name");
    recurring.amount  = money_from_string(req.get_param_value("input_amount"));
    recurring.recurs  = req.get_param_value("input_recurs");

    if (recurring.recurs != "monthly" && recurring.recurs != "weekly") {
        return api_error(req, res, "Invalid recurring frequency");
    }

    edit_recurring(recurring, previous_recurring);

    api_success(req, res, "Recurring " + to_string(recurring.id) + " has been modified");
}

void budget::delete_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("input_id")) {
        return api_error(req, res, "Invalid parameters");
    }

    auto id = req.get_param_value("input_id");

    if (!recurring_exists(budget::to_number<size_t>(id))) {
        return api_error(req, res, "The recurring " + id + " does not exit");
    }

    recurring_delete(budget::to_number<size_t>(id));

    api_success(req, res, "Recurring " + id + " has been deleted");
}

void budget::list_recurrings_api(const httplib::Request& req, httplib::Response& res) {
    std::stringstream ss;

    for (auto& recurring : all_recurrings()) {
        data_writer writer;
        recurring.save(writer);
        ss << writer.to_string() << std::endl;
    }

    api_success_content(req, res, ss.str());
}
