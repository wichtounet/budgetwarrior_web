//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <numeric>

#include "data_cache.hpp"

#include "writer.hpp"
#include "pages/expenses_pages.hpp"
#include "http.hpp"
#include "config.hpp"

#include <array>

using namespace budget;

namespace {

std::vector<std::pair<std::string, budget::money>> sort_map(const std::map<std::string, budget::money> & expense_sum, size_t max) {
    std::vector<std::pair<std::string, budget::money>> sorted_expenses;

    for (auto& [name, amount] : expense_sum) {
        sorted_expenses.emplace_back(name, amount);
    }

    std::sort(sorted_expenses.begin(), sorted_expenses.end(), [](auto& lhs, auto& rhs) {
        return lhs.second > rhs.second;
    });

    if (sorted_expenses.size() > max) {
        sorted_expenses.resize(max);
    }

    return sorted_expenses;
}


} // end of anonymous namespace

void budget::month_breakdown_expenses_graph(budget::html_writer& w, const std::string& title, budget::month month, budget::year year, bool mono, const std::string& style) {
    if (mono) {
        w.defer_script(R"=====(
            breakdown_expense_colors = (function () {
                var colors = [], base = Highcharts.getOptions().colors[3], i;
                for (i = 0; i < 10; i += 1) {
                    colors.push(Highcharts.Color(base).brighten((i - 3) / 7).get());
                }
                return colors;
            }());
        )=====");
    }

    data_cache cache;

    // standard breakdown per category
    {
        auto ss = start_chart_base(w, "pie", "month_breakdown_expense_categories_graph", style);

        ss << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

        if (mono) {
            ss << R"=====(plotOptions: {pie: { dataLabels: {enabled: false},  colors: breakdown_expense_colors, innerSize: '60%' }},)=====";
        }

        ss << "series: [";

        ss << "{ name: 'Expenses',";
        ss << "colorByPoint: true,";
        ss << "data: [";

        std::map<size_t, budget::money> account_sum;

        for (auto& expense : all_expenses_month(cache, year, month)) {
            account_sum[expense.account] += expense.amount;
        }

        budget::money total;

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

            ss << R"=====(<span class="text-danger">)=====";
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

    // standard breakdown per expense
    if (!mono) {
        auto ss = start_chart_base(w, "pie", "month_breakdown_expenses_graph", style);

        ss << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

        ss << "series: [";

        ss << "{ name: 'Expenses',";
        ss << "colorByPoint: true,";
        ss << "data: [";

        std::map<std::string, budget::money> expense_sum;

        for (auto& expense : all_expenses_month(cache, year, month)) {
            expense_sum[expense.name] += expense.amount;
        }

        auto sorted_expenses = sort_map(expense_sum, 20);

        for (auto& [name, amount] : sorted_expenses) {
            ss << "{";
            ss << "name: '" << name << "',";
            ss << "y: " << budget::money_to_string(amount);
            ss << "},";
        }

        ss << "]},";

        ss << "],";

        ss << R"=====(title: {text: ')=====";
        ss << title;
        ss << R"=====('},)=====";

        end_chart(w, ss);
    }

    // standard breakdown per group
    if (!mono) {
        std::string separator = config_value("aggregate_separator", "/");

        auto ss = start_chart_base(w, "pie", "month_breakdown_expenses_group_graph", style);

        ss << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

        ss << "series: [";

        ss << "{ name: 'Expenses',";
        ss << "colorByPoint: true,";
        ss << "data: [";

        std::map<std::string, budget::money> expense_sum;

        for (auto& expense : all_expenses_month(cache, year, month)) {
            auto name = expense.name;

            if (name[name.size() - 1] == ' ') {
                name.erase(name.size() - 1, name.size());
            }

            auto loc = name.find(separator);
            if (loc != std::string::npos) {
                name = name.substr(0, loc);
            }

            expense_sum[name] += expense.amount;
        }

        auto sorted_expenses = sort_map(expense_sum, 15);

        for (auto& [name, amount] : sorted_expenses) {
            ss << "{";
            ss << "name: '" << name << "',";
            ss << "y: " << budget::money_to_string(amount);
            ss << "},";
        }

        ss << "]},";

        ss << "],";

        ss << R"=====(title: {text: ')=====";
        ss << title;
        ss << R"=====('},)=====";

        end_chart(w, ss);
    }
}


void budget::expenses_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Expenses")) {
        return;
    }

    budget::html_writer w(content_stream);

    if (req.matches.size() == 3) {
        show_expenses(to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]), w);
    } else {
        show_expenses(w);
    }

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::search_expenses_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Search Expenses")) {
        return;
    }

    budget::html_writer w(content_stream);

    page_form_begin(w, "/expenses/search/");

    add_name_picker(w);

    form_end(w);

    if(req.has_param("input_name")){
        auto search = req.get_param_value("input_name");

        search_expenses(search, w);
    }

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::time_graph_expenses_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Expenses over time")) {
        return;
    }

    budget::html_writer w(content_stream);

    auto ss = start_time_chart(w, "Expenses over time", "line", "expenses_time_graph", "");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Monthly Expenses' }},)=====";
    ss << R"=====(legend: { enabled: false },)=====";

    ss << "series: [";

    ss << "{ name: 'Monthly expenses',";
    ss << "data: [";

    std::vector<budget::money> serie;
    std::vector<std::string> dates;

    data_cache cache;

    auto sy = start_year();

    for(unsigned short j = sy; j <= budget::local_day().year(); ++j){
        budget::year year = j;

        auto sm = start_month(year);
        auto last = 13;

        if(year == budget::local_day().year()){
            last = budget::local_day().month() + 1;
        }

        for(unsigned short i = sm; i < last; ++i){
            budget::month month = i;

            budget::money sum;

            for (auto& expense : all_expenses_month(cache, year, month)) {
                sum += expense.amount;
            }

            std::string date = "Date.UTC(" + std::to_string(year) + "," + std::to_string(month.value - 1) + ", 1)";

            serie.push_back(sum);
            dates.push_back(date);

            ss << "[" << date <<  "," << budget::money_to_string(sum) << "],";
        }
    }

    ss << "]},";

    add_average_12_serie(ss, serie, dates);

    ss << "]";

    end_chart(w, ss);

    // If configured as such, we create a second graph without taxes

    if (config_contains("taxes_account")) {
        auto taxes_account = config_value("taxes_account");

        if (account_exists(taxes_account)) {
            auto ss = start_time_chart(w, "Expenses w/o taxes over time", "line", "expenses_no_taxes_time_graph", "");

            ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
            ss << R"=====(yAxis: { min: 0, title: { text: 'Monthly Expenses W/O Taxes' }},)=====";
            ss << R"=====(legend: { enabled: false },)=====";

            ss << "series: [";

            ss << "{ name: 'Monthly expenses W/O Taxes',";
            ss << "data: [";

            std::vector<budget::money> serie;
            std::vector<std::string> dates;

            auto sy = start_year();

            for (unsigned short j = sy; j <= budget::local_day().year(); ++j) {
                budget::year year = j;

                auto sm   = start_month(year);
                auto last = 13;

                if (year == budget::local_day().year()) {
                    last = budget::local_day().month() + 1;
                }

                for (unsigned short i = sm; i < last; ++i) {
                    budget::month month = i;

                    budget::money sum;

                    for (auto& expense : all_expenses_month(cache, year, month)) {
                        if (get_account(expense.account).name != taxes_account) {
                            sum += expense.amount;
                        }
                    }

                    std::string date = "Date.UTC(" + std::to_string(year) + "," + std::to_string(month.value - 1) + ", 1)";

                    serie.push_back(sum);
                    dates.push_back(date);

                    ss << "[" << date << "," << budget::money_to_string(sum) << "],";
                }
            }

            ss << "]},";

            add_average_12_serie(ss, serie, dates);

            ss << "]";

            end_chart(w, ss);
        }
    }

    page_end(w, req, res);
}

void budget::all_expenses_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "All Expenses")) {
        return;
    }

    budget::html_writer w(content_stream);
    budget::show_all_expenses(w);

    make_tables_sortable(w);

    page_end(w, req, res);
}

void budget::month_breakdown_expenses_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Expenses Breakdown")) {
        return;
    }

    auto today = budget::local_day();

    auto month = today.month();
    auto year  = today.year();

    if (req.matches.size() == 3) {
        year  = to_number<size_t>(req.matches[1]);
        month = to_number<size_t>(req.matches[2]);
    }

    budget::html_writer w(content_stream);

    w << title_begin << "Expenses Breakdown of " << month << " " << year << budget::year_month_selector{"expenses/breakdown/month", year, month} << title_end;

    month_breakdown_expenses_graph(w, "Expenses Breakdown", month, year);

    page_end(w, req, res);
}

void budget::year_breakdown_expenses_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Expenses Breakdown")) {
        return;
    }

    auto today = budget::local_day();

    auto year = today.year();

    if (req.matches.size() == 2) {
        year = to_number<size_t>(req.matches[1]);
    }

    budget::html_writer w(content_stream);

    w << title_begin << "Expense Categories Breakdown of " << year << budget::year_selector{"expenses/breakdown/year", year} << title_end;

    data_cache cache;

    {
        auto ss = start_chart(w, "Expense Categories Breakdown", "pie", "category_pie");

        ss << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";
        ss << R"=====(accessibility: { point: { valueSuffix: '%' } },)=====";
        ss << R"=====(plotOptions: { pie: { showInLegend: true } },)=====";

        ss << "series: [";

        ss << "{ name: 'Expenses',";
        ss << "colorByPoint: true,";
        ss << "data: [";

        std::map<std::string, budget::money> account_sum;

        for (auto& expense : all_expenses_year(cache, year)) {
            account_sum[get_account(expense.account).name] += expense.amount;
        }

        for (auto& [name, amount] : account_sum) {
            ss << "{";
            ss << "name: '" << name << "',";
            ss << "y: " << budget::money_to_string(amount);
            ss << "},";
        }

        ss << "]},";

        ss << "]";

        end_chart(w, ss);
    }

    {
        auto breakdown_ss = start_chart(w, "Expenses Breakdown", "pie", "expenses_chart");

        breakdown_ss << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";
        breakdown_ss << R"=====(accessibility: { point: { valueSuffix: '%' } },)=====";
        breakdown_ss << R"=====(plotOptions: { pie: { showInLegend: true } },)=====";

        breakdown_ss << "series: [";

        breakdown_ss << "{ name: 'Expenses',";
        breakdown_ss << "colorByPoint: true,";
        breakdown_ss << "data: [";

        std::map<std::string, budget::money> expense_sum;

        for (auto& expense : all_expenses_year(cache, year)) {
            expense_sum[expense.name] += expense.amount;
        }

        auto sorted_expenses = sort_map(expense_sum, 20);

        for (auto& [name, amount] : sorted_expenses) {
            breakdown_ss << "{";
            breakdown_ss << "name: '" << name << "',";
            breakdown_ss << "y: " << budget::money_to_string(amount);
            breakdown_ss << "},";
        }

        breakdown_ss << "]},";

        breakdown_ss << "]";

        end_chart(w, breakdown_ss);
    }

    {
        std::string separator = config_value("aggregate_separator", "/");

        auto aggregate_ss = start_chart(w, "Aggregate Expenses Breakdown", "pie", "aggregate_pie");

        aggregate_ss << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";
        aggregate_ss << R"=====(accessibility: { point: { valueSuffix: '%' } },)=====";
        aggregate_ss << R"=====(plotOptions: { pie: { showInLegend: true } },)=====";

        aggregate_ss << "series: [";

        aggregate_ss << "{ name: 'Expenses',";
        aggregate_ss << "colorByPoint: true,";
        aggregate_ss << "data: [";

        std::map<std::string, budget::money> expense_sum;

        for (auto& expense : all_expenses_year(cache, year)) {
            auto name = expense.name;

            if (name[name.size() - 1] == ' ') {
                name.erase(name.size() - 1, name.size());
            }

            auto loc = name.find(separator);
            if (loc != std::string::npos) {
                name = name.substr(0, loc);
            }

            expense_sum[name] += expense.amount;
        }

        auto sorted_expenses = sort_map(expense_sum, 15);

        for (auto& [name, amount] : sorted_expenses) {
            aggregate_ss << "{";
            aggregate_ss << "name: '" << name << "',";
            aggregate_ss << "y: " << budget::money_to_string(amount);
            aggregate_ss << "},";
        }

        aggregate_ss << "]},";

        aggregate_ss << "]";

        end_chart(w, aggregate_ss);
    }

    page_end(w, req, res);
}

void budget::add_expenses_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "New Expense")) {
        return;
    }

    budget::html_writer w(content_stream);

    w << title_begin << "New Expense" << title_end;

    form_begin(w, "/api/expenses/add/", "/expenses/add/");

    add_date_picker(w);
    add_name_picker(w);
    add_amount_picker(w);

    std::string account;

    if (config_contains("default_account")) {
        auto default_account = config_value("default_account");

        if (account_exists(default_account)) {
            auto today = budget::local_day();
            account = budget::to_string(get_account(default_account, today.year(), today.month()).id);
        }
    }

    add_account_picker(w, budget::local_day(), account);

    form_end(w);

    page_end(w, req, res);
}

void budget::edit_expenses_page(const httplib::Request& req, httplib::Response& res) {
    std::stringstream content_stream;
    if (!page_start(req, res, content_stream, "Edit Expense")) {
        return;
    }

    budget::html_writer w(content_stream);

    if (!req.has_param("input_id") || !req.has_param("back_page")) {
        display_error_message(w, "Invalid parameter for the request");
    } else {
        auto input_id = req.get_param_value("input_id");

        if (!expense_exists(budget::to_number<size_t>(input_id))) {
            display_error_message(w, "The expense " + input_id + " does not exist");
        } else {
            auto back_page = req.get_param_value("back_page");

            w << title_begin << "Edit Expense " << input_id << title_end;

            form_begin_edit(w, "/api/expenses/edit/", back_page, input_id);

            auto expense = expense_get(budget::to_number<size_t>(input_id));

            add_date_picker(w, budget::to_string(expense.date));
            add_name_picker(w, expense.name);
            add_amount_picker(w, budget::money_to_string(expense.amount));
            add_account_picker(w, expense.date, budget::to_string(expense.account));

            form_end(w);
        }
    }

    page_end(w, req, res);
}
