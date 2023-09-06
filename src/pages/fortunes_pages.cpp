//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "fortune.hpp"

#include "pages/fortunes_pages.hpp"
#include "pages/html_writer.hpp"
#include "http.hpp"

using namespace budget;

void budget::list_fortunes_page(html_writer & w) {
    budget::list_fortunes(w);

    make_tables_sortable(w);
}

void budget::graph_fortunes_page(html_writer & w) {
    auto ss = start_chart(w, "Fortune", "spline");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Fortune' }},)=====";

    ss << "series: [";

    ss << "{ name: 'Fortune',";
    ss << "data: [";

    auto sorted_fortunes = all_fortunes();

    std::ranges::sort(sorted_fortunes,
              [](const budget::fortune& a, const budget::fortune& b) { return a.check_date < b.check_date; });

    for (auto& value : sorted_fortunes) {
        auto& date = value.check_date;

        ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::money_to_string(value.amount) << "],";
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);
}

void budget::status_fortunes_page(html_writer & w) {
    budget::status_fortunes(w, false);

    make_tables_sortable(w);
}

void budget::add_fortunes_page(html_writer & w) {
    w << title_begin << "New fortune" << title_end;

    form_begin(w, "/api/fortunes/add/", "/fortunes/add/");

    add_date_picker(w);
    add_amount_picker(w);

    form_end(w);
}

void budget::edit_fortunes_page(html_writer & w, const httplib::Request& req) {
    if (!validate_parameters(w, req, {"input_id", "back_page"})){
        return;
    }

    auto input_id = req.get_param_value("input_id");

    if (!fortune_exists(budget::to_number<size_t>(input_id))) {
        return display_error_message(w, "The fortune {} does not exist", input_id);
    }

    auto back_page = req.get_param_value("back_page");

    w << title_begin << "Edit fortune " << input_id << title_end;

    form_begin_edit(w, "/api/fortunes/edit/", back_page, input_id);

    auto fortune = fortune_get(budget::to_number<size_t>(input_id));

    add_date_picker(w, budget::to_string(fortune.check_date));
    add_amount_picker(w, budget::money_to_string(fortune.amount));

    form_end(w);
}
