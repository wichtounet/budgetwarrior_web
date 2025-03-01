//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "retirement.hpp"
#include "config.hpp"
#include "assets.hpp"
#include "pages/html_writer.hpp"
#include "pages/retirement_pages.hpp"
#include "pages/web_config.hpp"
#include "http.hpp"

using namespace budget;

namespace {

void add_percent_picker(budget::writer& w, std::string_view title, std::string_view name, double default_value = 0.0) {
    w << R"=====(<div class="form-group">)=====";

    w << "<label for=\"" << name << "\">" << title << "</label>";
    w << R"(<input required type="number" min="0" max="100" step="0.01" class="form-control" id=")" << name << "\" name=\"" << name << "\" ";
    w << " value=\"" << default_value << "\" ";
    w << R"=====(
            >
         </div>
    )=====";
}

} // namespace

void budget::retirement_status_page(html_writer& w) {
    w << title_begin << "Retirement status" << title_end;

    if (!internal_config_contains("withdrawal_rate")) {
        return display_error_message(w, "Not enough information, please configure Retirement Options first");
    }

    if (!internal_config_contains("expected_roi")) {
        return display_error_message(w, "Not enough information, please configure Retirement Options first");
    }

    budget::retirement_status(w);
}

void budget::retirement_configure_page(html_writer& w) {
    w << title_begin << "Retirement Options" << title_end;

    form_begin(w, "/api/retirement/configure/", "/retirement/status/");

    if (!internal_config_contains("withdrawal_rate")) {
        add_percent_picker(w, "Withdrawal Rate [%]", "input_wrate", 4.0);
    } else {
        add_percent_picker(w, "Withdrawal Rate [%]", "input_wrate", to_number<double>(internal_config_value("withdrawal_rate")));
    }

    if (!internal_config_contains("expected_roi")) {
        add_percent_picker(w, "Annual Return [%]", "input_roi", 5.0);
    } else {
        add_percent_picker(w, "Annual Return [%]", "input_roi", to_number<double>(internal_config_value("expected_roi")));
    }

    form_end(w);
}

void budget::retirement_fi_ratio_over_time(html_writer& w) {
    if (no_assets() || no_asset_values()) {
        return;
    }

    auto ss = start_time_chart(w, "FI Ratio over time", "line", "fi_time_graph", "");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'FI Ratio' }},)=====";
    ss << R"=====(legend: { enabled: false },)=====";

    ss << "series: [";

    ss << "{ name: 'FI Ratio (Current Expenses)',";
    ss << "data: [";

    {
        auto date     = budget::asset_start_date(w.cache);
        auto end_date = budget::local_day();

        while (date <= end_date) {
            const auto ratio = budget::fi_ratio(date, w.cache);

            ss << std::format("[Date.UTC({},{},{}), {}],", date.year().value, date.month().value - 1, date.day().value, 100 * ratio);

            date += days(1);
        }

        ss << "]},";
    }

    if (auto fixed_expenses = budget::get_fi_expenses(); fixed_expenses) {
        ss << "{ name: 'FI Ratio (" << fixed_expenses.dollars() << ' ' << get_default_currency() << " yearly expenses)',";
        ss << "data: [";

        auto date     = budget::asset_start_date(w.cache);
        auto end_date = budget::local_day();

        while (date <= end_date) {
            const auto ratio = budget::fixed_fi_ratio(date, w.cache, fixed_expenses);

            ss << std::format("[Date.UTC({},{},{}), {}],", date.year().value, date.month().value - 1, date.day().value, 100 * ratio);

            date += days(1);
        }

        ss << "]},";
    }

    ss << "]";

    end_chart(w, ss);
}
