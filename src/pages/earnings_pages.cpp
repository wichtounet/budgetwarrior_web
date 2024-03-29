//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <numeric>
#include <array>

#include "cpp_utils/hash.hpp"

#include "data_cache.hpp"
#include "date.hpp"
#include "http.hpp"
#include "config.hpp"
#include "views.hpp"

#include "pages/html_writer.hpp"
#include "pages/earnings_pages.hpp"
#include "pages/web_config.hpp"

using namespace budget;

void budget::month_breakdown_income_graph(
        budget::html_writer& w, std::string_view title, budget::month month, budget::year year, bool mono, std::string_view style) {
    if (mono) {
        w.defer_script(R"=====(
            breakdown_income_colors = (function () {
                var colors = [], base = Highcharts.getOptions().colors[0], i;
                for (i = 0; i < 10; i += 1) {
                    colors.push(Highcharts.Color(base).brighten((i - 3) / 7).get());
                }
                return colors;
            }());
        )=====");
    }

    auto ss = start_chart_base(w, "pie", "month_breakdown_income_graph", style);

    ss << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    if (mono) {
        ss << R"=====(plotOptions: { pie: { dataLabels: {enabled: false},  colors: breakdown_income_colors, innerSize: '60%' }},)=====";
    }

    ss << "series: [";

    ss << "{ name: 'Income',";
    ss << "colorByPoint: true,";
    ss << "data: [";

    std::map<size_t, budget::money, std::less<>> account_sum;

    for (auto& earning : all_earnings_month(w.cache, year, month)) {
        account_sum[earning.account] += earning.amount;
    }

    budget::money total = get_base_income(w.cache);

    if (total) {
        ss << "{";
        ss << "name: 'Salary',";
        ss << "y: " << budget::money_to_string(total);
        ss << "},";
    }

    for (auto& [id, amount] : account_sum) {
        ss << "{";
        ss << "name: '" << get_account(id).name << "',";
        ss << "y: " << budget::money_to_string(amount);
        ss << "},";

        total += amount;
    }

    ss << "]},";

    ss << "],";

    if (mono) {
        ss << R"=====(title: {verticalAlign: 'middle', useHTML: true, text: ')=====";

        ss << R"=====(<div class="gauge-cash-flow-title"><strong>)=====";
        ss << title;
        ss << R"=====(</strong><br/><hr class="flat-hr" />)=====";

        ss << R"=====(<span class="text-success">)=====";
        ss << total << " __currency__";
        ss << R"=====(</span></div>)=====";
        ss << R"=====('},)=====";
    } else {
        ss << R"=====(title: {text: ')=====";
        ss << title;
        ss << R"=====('},)=====";
    }

    end_chart(w, ss);
}

void budget::time_graph_income_page(html_writer& w) {
    {
        auto ss = start_time_chart(w, "Income over time", "line", "income_time_graph", "");

        ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
        ss << R"=====(yAxis: { min: 0, title: { text: 'Monthly Income' }},)=====";
        ss << R"=====(legend: { enabled: false },)=====";

        ss << "series: [";

        ss << "{ name: 'Monthly income',";
        ss << "data: [";

        std::vector<budget::money> serie;
        std::vector<std::string>   dates;

        auto sy = start_year(w.cache);

        for (budget::year year = sy; year <= budget::local_day().year(); ++year) {
            const auto sm   = start_month(w.cache, year);
            const auto last = last_month(year);

            for (budget::month month = sm; month < last; ++month) {
                auto sum = get_base_income(w.cache, budget::date(year, month, 2)) + fold_left_auto(all_earnings_month(w.cache, year, month) | to_amount);

                const std::string date = std::format("Date.UTC({},{},1)", year.value, month.value - 1);

                serie.push_back(sum);
                dates.push_back(date);

                ss << "[" << date << "," << budget::money_to_string(sum) << "],";
            }
        }

        ss << "]},";

        add_average_12_serie(ss, serie, dates);
        add_average_24_serie(ss, serie, dates);

        ss << "]";

        end_chart(w, ss);
    }

    {
        auto ss = start_time_chart(w, "Annual Income over time", "line", "annual_income_time_graph", "");

        ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
        ss << R"=====(yAxis: { min: 0, title: { text: 'Yearly Income' }},)=====";
        ss << R"=====(legend: { enabled: false },)=====";

        ss << "series: [";

        ss << "{ name: 'Yearly income',";
        ss << "data: [";

        std::vector<budget::money> serie;
        std::vector<std::string>   dates;

        auto sy = start_year(w.cache);

        for (budget::year year = sy; year <= budget::local_day().year(); ++year) {
            const auto sm   = start_month(w.cache, year);
            const auto last = last_month(year);

            budget::money sum;

            for (budget::month m = sm; m < last; ++m) {
                sum += get_base_income(w.cache, budget::date(year, m, 2)) + fold_left_auto(all_earnings_month(w.cache, year, m) | to_amount);
            }

            const std::string date = std::format("Date.UTC({},1,1)", year.value);

            serie.push_back(sum);
            dates.push_back(date);

            ss << "[" << date << "," << budget::money_to_string(sum) << "],";
        }

        ss << "]},";

        add_average_5_serie(ss, serie, dates);

        ss << "]";

        end_chart(w, ss);
    }
}

void budget::time_graph_earnings_page(html_writer& w) {
    auto ss = start_time_chart(w, "Earnings over time", "line", "earnings_time_graph", "");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Monthly Earnings' }},)=====";
    ss << R"=====(legend: { enabled: false },)=====";

    ss << "series: [";

    ss << "{ name: 'Monthly earnings',";
    ss << "data: [";

    auto sy = start_year(w.cache);

    for (budget::year year = sy; year <= budget::local_day().year(); ++year) {
        const auto sm   = start_month(w.cache, year);
        const auto last = last_month(year);

        for (budget::month month = sm; month < last; ++month) {
            const auto sum = fold_left_auto(all_earnings_month(w.cache, year, month) | to_amount);

            ss << "[Date.UTC(" << year << "," << month.value - 1 << ", 1) ," << budget::money_to_string(sum) << "],";
        }
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);
}

namespace {

void add_quick_earning_action(budget::html_writer& w, size_t i, const budget::earning& earning) {
    w << "<script>";
    w << "function quickAction" << i << "() {";
    w << R"(  $("#input_name").val(")" << earning.name << "\");";
    w << "  $(\"#input_amount\").val(" << budget::to_string(earning.amount) << ");";
    w << "  $(\"#input_account\").val(" << earning.account << ");";
    w << "}";
    w << "</script>";
    w << R"(<button class="btn btn-secondary" onclick="quickAction)" << i << "();\">" << earning.name << "</button>&nbsp;";
}

} // end of anonymous namespace

void budget::add_earnings_page(html_writer& w) {
    w << title_begin << "New earning" << title_end;

    if (w.cache.earnings().size() > quick_actions) {
        std::map<std::string, size_t, std::less<>>                    counts;
        cpp::string_hash_map<budget::earning> last_earnings;
        std::vector<std::pair<std::string, size_t>>      order;

        for (const auto& earning : w.cache.sorted_earnings()) {
            ++counts[earning.name];
            last_earnings[earning.name] = earning;
        }

        order.reserve(counts.size());
        for (auto& [key, value] : counts) {
            order.emplace_back(key, value);
        }

        std::ranges::sort(order, [](const auto& a, const auto& b) { return a.second > b.second; });

        w << "<div>";
        w << "Quick Fill: ";
        for (size_t i = 0; i < quick_actions && i < order.size(); ++i) {
            add_quick_earning_action(w, i, last_earnings[order[i].first]);
        }
        w << "</div>";
    }

    form_begin(w, "/api/earnings/add/", "/earnings/add/");

    add_date_picker(w);
    add_name_picker(w);
    add_amount_picker(w);

    std::string account;
    if (has_default_account()) {
        account = budget::to_string(default_account().id);
    }

    add_account_picker(w, budget::local_day(), account);

    form_end(w);
}

void budget::edit_earnings_page(html_writer& w, const httplib::Request& req) {
    if (!req.has_param("input_id") || !req.has_param("back_page")) {
        return display_error_message(w, "Invalid parameter for the request");
    }

    auto input_id = req.get_param_value("input_id");
    if (!earning_exists(budget::to_number<size_t>(input_id))) {
        return display_error_message(w, "The earning {} does not exist", input_id);
    }

    auto back_page = req.get_param_value("back_page");

    w << title_begin << "Edit earning " << input_id << title_end;

    form_begin_edit(w, "/api/earnings/edit/", back_page, input_id);

    auto earning = earning_get(budget::to_number<size_t>(input_id));

    add_date_picker(w, budget::to_string(earning.date));
    add_name_picker(w, earning.name);
    add_amount_picker(w, budget::money_to_string(earning.amount));
    add_account_picker(w, earning.date, budget::to_string(earning.account));

    form_end(w);
}

void budget::earnings_page(html_writer& w, const httplib::Request& req) {
    if (req.matches.size() == 3) {
        show_earnings(month_from_string(req.matches[2]), year_from_string(req.matches[1]), w);
    } else {
        show_earnings(w);
    }

    make_tables_sortable(w);
}

void budget::all_earnings_page(html_writer& w) {
    budget::show_all_earnings(w);

    make_tables_sortable(w);
}

void budget::search_earnings_page(html_writer& w, const httplib::Request& req) {
    page_form_begin(w, "/earnings/search/");

    add_name_picker(w);

    form_end(w);

    if (req.has_param("input_name")) {
        auto search = req.get_param_value("input_name");

        search_earnings(search, w);
    }

    make_tables_sortable(w);
}
