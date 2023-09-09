//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "pages/objectives_pages.hpp"
#include "pages/html_writer.hpp"
#include "http.hpp"
#include "config.hpp"

using namespace budget;

namespace {

void add_objective_operator_picker(budget::writer& w, std::string_view default_value = "") {
    w << R"=====(
            <div class="form-group">
                <label for="input_operator">Operator</label>
                <select class="form-control" id="input_operator" name="input_operator">
    )=====";

    if ("min" == default_value) {
        w << "<option selected value=\"min\">Min</option>";
    } else {
        w << "<option value=\"min\">Min</option>";
    }

    if ("max" == default_value) {
        w << "<option selected value=\"max\">Max</option>";
    } else {
        w << "<option value=\"max\">Max</option>";
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void add_objective_type_picker(budget::writer& w, std::string_view default_value = "") {
    w << R"=====(
            <div class="form-group">
                <label for="input_type">Type</label>
                <select class="form-control" id="input_type" name="input_type">
    )=====";

    if ("monthly" == default_value) {
        w << "<option selected value=\"monthly\">Monthly</option>";
    } else {
        w << "<option value=\"monthly\">Monthly</option>";
    }

    if ("yearly" == default_value) {
        w << "<option selected value=\"yearly\">Yearly</option>";
    } else {
        w << "<option value=\"yearly\">Yearly</option>";
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void add_objective_source_picker(budget::writer& w, std::string_view default_value = "") {
    w << R"=====(
            <div class="form-group">
                <label for="input_source">Source</label>
                <select class="form-control" id="input_source" name="input_source">
    )=====";

    if ("balance" == default_value) {
        w << "<option selected value=\"balance\">Balance</option>";
    } else {
        w << "<option value=\"balance\">Balance</option>";
    }

    if ("earnings" == default_value) {
        w << "<option selected value=\"earnings\">Earnings</option>";
    } else {
        w << "<option value=\"earnings\">Earnings</option>";
    }

    if ("income" == default_value) {
        w << "<option selected value=\"income\">Income</option>";
    } else {
        w << "<option value=\"income\">Income</option>";
    }

    if ("expenses" == default_value) {
        w << "<option selected value=\"expenses\">Expenses</option>";
    } else {
        w << "<option value=\"expenses\">Expenses</option>";
    }

    if ("expenses_no_taxes" == default_value) {
        w << "<option selected value=\"expenses_no_taxes\">Expenses w/o taxes</option>";
    } else {
        w << "<option value=\"expenses_no_taxes\">Expenses w/o taxes</option>";
    }

    if ("savings_rate" == default_value) {
        w << "<option selected value=\"savings_rate\">Savings Rate</option>";
    } else {
        w << "<option value=\"savings_rate\">Savings Rate</option>";
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

} // namespace

void budget::objectives_card(budget::html_writer& w) {
    // if the user does not use objectives, this card does not make sense
    if (w.cache.objectives().empty()) {
        return;
    }

    const auto today = budget::local_day();

    const auto m = today.month();
    const auto y = today.year();

    // Compute the year/month status
    auto year_status  = budget::compute_year_status(w.cache);
    auto month_status = budget::compute_month_status(w.cache, y, m);

    w << R"=====(<div class="card">)=====";
    w << R"=====(<div class="card-header card-header-primary">Goals</div>)=====";

    w << R"=====(<div class="row card-body">)=====";

    for (size_t i = 0; i < w.cache.objectives().size(); ++i) {
        auto& objective = w.cache.objectives()[i];

        w << R"=====(<div class="col-lg-2 col-md-3 col-sm-4 col-xs-6">)=====";

        auto ss = start_chart_base(w, "solidgauge", "objective_gauge_" + budget::to_string(i), "height: 200px");

        ss << R"=====(title: {style: {color: "rgb(124, 181, 236)", fontWeight: "bold" }, text: ')=====";
        ss << objective.name;
        ss << R"=====('},)=====";

        ss << R"=====(tooltip: { enabled: false },)=====";
        ss << R"=====(yAxis: { min: 0, max: 100, lineWidth: 0, tickPositions: [], },)=====";

        std::string status;
        int         success_int = 0;

        if (objective.type == "yearly") {
            status      = budget::get_status(year_status, objective);
            success_int = budget::compute_success(year_status, objective);
        } else if (objective.type == "monthly") {
            status      = budget::get_status(month_status, objective);
            success_int = budget::compute_success(month_status, objective);
        } else {
            throw budget_exception("Invalid objective type", true);
        }

        ss << R"=====(plotOptions: {)=====";
        ss << R"=====(solidgauge: {)=====";

        ss << R"=====(dataLabels: {)=====";
        ss << R"=====(enabled: true, verticalAlign: "middle", borderWidth: 0, useHTML: true, )=====";

        ss << R"=====(format: '<div class="gauge-objective-title"><span class="lead""><strong>)=====";
        ss << success_int;
        ss << R"=====(%</strong></span> <br />)=====";
        ss << status;
        ss << R"=====(</div>')=====";

        ss << R"=====(},)=====";

        ss << R"=====(rounded: true)=====";
        ss << R"=====(})=====";
        ss << R"=====(},)=====";

        ss << R"=====(series: [{)=====";
        ss << "name: '" << objective.name << "',";
        ss << R"=====(data: [{)=====";
        ss << R"=====(radius: '112%',)=====";
        ss << R"=====(innerRadius: '88%',)=====";
        ss << "y: " << std::min(success_int, 100);
        ss << R"=====(}])=====";
        ss << R"=====(}])=====";

        end_chart(w, ss);

        w << R"=====(</div>)=====";
    }

    w << R"=====(</div>)=====";
    w << R"=====(</div>)=====";
}

void budget::list_objectives_page(html_writer& w) {
    budget::list_objectives(w);

    make_tables_sortable(w);
}

void budget::status_objectives_page(html_writer& w) {
    budget::status_objectives(w);
}

void budget::add_objectives_page(html_writer& w) {
    w << title_begin << "New objective" << title_end;

    form_begin(w, "/api/objectives/add/", "/objectives/add/");

    add_name_picker(w);
    add_objective_type_picker(w);
    add_objective_source_picker(w);
    add_objective_operator_picker(w);
    add_amount_picker(w);

    form_end(w);
}

void budget::edit_objectives_page(html_writer& w, const httplib::Request& req) {
    if (!validate_parameters(w, req, {"input_id", "back_page"})) {
        return;
    }

    auto input_id = req.get_param_value("input_id");
    if (!objective_exists(budget::to_number<size_t>(input_id))) {
        return display_error_message(w, "The objective {} does not exist", input_id);
    }

    auto back_page = req.get_param_value("back_page");

    w << title_begin << "Edit objective " << input_id << title_end;

    form_begin_edit(w, "/api/objectives/edit/", back_page, input_id);

    auto objective = objective_get(budget::to_number<size_t>(input_id));

    add_name_picker(w, objective.name);
    add_objective_type_picker(w, objective.type);
    add_objective_source_picker(w, objective.source);
    add_objective_operator_picker(w, objective.op);
    add_amount_picker(w, budget::money_to_string(objective.amount));

    form_end(w);
}
