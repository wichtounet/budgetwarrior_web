//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <numeric>

#include "overview.hpp"
#include "data_cache.hpp"

#include "pages/html_writer.hpp"
#include "pages/overview_pages.hpp"
#include "http.hpp"
#include "config.hpp"
#include "compute.hpp"

using namespace budget;

void budget::overview_page(html_writer & w, const httplib::Request& req) {
    if (req.matches.size() == 3) {
        display_month_overview(to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]), w);
    } else {
        auto today = budget::local_day();
        display_month_overview(today.month(), today.year(), w);
    }
}

void budget::overview_aggregate_all_page(html_writer & w) {
    // Configuration of the overview
    bool full             = config_contains_and_true("aggregate_full");
    bool disable_groups   = config_contains_and_true("aggregate_no_group");
    std::string separator = config_value("aggregate_separator", "/");

    aggregate_all_overview(w, full, disable_groups, separator);
}

void budget::overview_aggregate_year_page(html_writer & w, const httplib::Request& req) {
    // Configuration of the overview
    bool full             = config_contains_and_true("aggregate_full");
    bool disable_groups   = config_contains_and_true("aggregate_no_group");
    std::string separator = config_value("aggregate_separator", "/");

    if (req.matches.size() == 2) {
        aggregate_year_overview(w, full, disable_groups, separator, to_number<size_t>(req.matches[1]));
    } else {
        auto today = budget::local_day();
        aggregate_year_overview(w, full, disable_groups, separator, today.year());
    }
}

void budget::overview_aggregate_year_fv_page(html_writer & w, const httplib::Request& req) {
    // Configuration of the overview
    bool full             = config_contains_and_true("aggregate_full");
    bool disable_groups   = config_contains_and_true("aggregate_no_group");
    std::string separator = config_value("aggregate_separator", "/");

    if (req.matches.size() == 2) {
        aggregate_year_fv_overview(w, full, disable_groups, separator, to_number<size_t>(req.matches[1]));
    } else {
        auto today = budget::local_day();
        aggregate_year_fv_overview(w, full, disable_groups, separator, today.year());
    }
}

void budget::overview_aggregate_year_month_page(html_writer & w, const httplib::Request& req) {
    // Configuration of the overview
    bool full             = config_contains_and_true("aggregate_full");
    bool disable_groups   = config_contains_and_true("aggregate_no_group");
    std::string separator = config_value("aggregate_separator", "/");

    if (req.matches.size() == 2) {
        aggregate_year_month_overview(w, full, disable_groups, separator, to_number<size_t>(req.matches[1]));
    } else {
        auto today = budget::local_day();
        aggregate_year_month_overview(w, full, disable_groups, separator, today.year());
    }
}

void budget::overview_aggregate_month_page(html_writer & w, const httplib::Request& req) {
    // Configuration of the overview
    bool full             = config_contains_and_true("aggregate_full");
    bool disable_groups   = config_contains_and_true("aggregate_no_group");
    std::string separator = config_value("aggregate_separator", "/");

    if (req.matches.size() == 3) {
        aggregate_month_overview(w, full, disable_groups, separator, to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]));
    } else {
        auto today = budget::local_day();
        aggregate_month_overview(w, full, disable_groups, separator, today.month(), today.year());
    }
}

void budget::overview_year_page(html_writer & w, const httplib::Request& req) {
    data_cache cache;

    budget::year year = budget::local_day().year();
    if (req.matches.size() == 2) {
        year = to_number<size_t>(req.matches[1]);
    }

    // Display the Summary Yearly Overview
    display_year_overview_header(cache, year, w);

    auto last = 13;
    if (year == budget::local_day().year()) {
        last = budget::local_day().month() + 1;
    }

    // Display the Yearly expense

    {
        auto ss = start_time_chart(w, "Expenses", "line", "year_overview_expenses_time_graph", "");

        ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
        ss << R"=====(yAxis: { min: 0, title: { text: 'Monthly Expenses' }},)=====";
        ss << R"=====(legend: { enabled: false },)=====";

        ss << "series: [";

        ss << "{ name: '" << year << " Expenses',";
        ss << "data: [";

        for (budget::month month = start_month(cache, year); month < last; ++month) {
            auto sum = accumulate_amount(all_expenses_month(cache, year, month));

            std::string date = "Date.UTC(" + std::to_string(year) + "," + std::to_string(month.value - 1) + ", 1)";
            ss << "[" << date << "," << budget::money_to_string(sum) << "],";
        }

        ss << "]},";

        if (year - 1 >= start_year(cache)) {
            ss << "{ name: '" << year - 1 << " Expenses',";
            ss << "data: [";

            for (budget::month month = start_month(cache, year - 1); month < 13; ++month) {
                auto sum = accumulate_amount(all_expenses_month(cache, year - 1, month));

                std::string date = "Date.UTC(" + std::to_string(year) + "," + std::to_string(month.value - 1) + ", 1)";
                ss << "[" << date << "," << budget::money_to_string(sum) << "],";
            }

            ss << "]},";
        }

        ss << "]";

        end_chart(w, ss);
    }

    // Display the Yearly Income

    {
        auto ss = start_time_chart(w, "Income", "line", "year_overview_income_time_graph", "");

        ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
        ss << R"=====(yAxis: { min: 0, title: { text: 'Monthly Income' }},)=====";
        ss << R"=====(legend: { enabled: false },)=====";

        ss << "series: [";

        ss << "{ name: '" << year << " Expenses',";
        ss << "data: [";

        for (budget::month month = start_month(cache, year); month < last; ++month) {
            auto sum = get_base_income(cache, budget::date(year, month, 2)) + accumulate_amount(all_earnings_month(cache, year, month));

            std::string date = "Date.UTC(" + std::to_string(year) + "," + std::to_string(month.value - 1) + ", 1)";
            ss << "[" << date << "," << budget::money_to_string(sum) << "],";
        }

        ss << "]},";

        if (year - 1 >= start_year(cache)) {
            ss << "{ name: '" << year - 1 << " Expenses',";
            ss << "data: [";

            for (budget::month month = start_month(cache, year - 1); month < 13; ++month) {
                auto sum = get_base_income(cache, budget::date(year - 1, month, 2)) + accumulate_amount(all_earnings_month(cache, year - 1, month));

                std::string date = "Date.UTC(" + std::to_string(year) + "," + std::to_string(month.value - 1) + ", 1)";
                ss << "[" << date << "," << budget::money_to_string(sum) << "],";
            }

            ss << "]},";
        }

        ss << "]";

        end_chart(w, ss);
    }

    // Display The Full Table Yearly Overview
    display_year_overview(cache, year, w);
}

void budget::time_graph_savings_rate_page(html_writer & w) {
    auto ss = start_time_chart(w, "Savings rate over time", "line", "savings_time_graph", "");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, max: 100, title: { text: 'Monthly Savings Rate' }},)=====";
    ss << R"=====(legend: { enabled: false },)=====";

    ss << "series: [";

    ss << "{ name: 'Savings Rate',";
    ss << "data: [";

    std::vector<float> serie;
    std::vector<std::string> dates;

    data_cache cache;

    auto sy = start_year(cache);

    for (unsigned short j = sy; j <= budget::local_day().year(); ++j) {
        budget::year year = j;

        auto sm = start_month(cache, year);
        auto last = 13;

        if (year == budget::local_day().year()) {
            last = budget::local_day().month() + 1;
        }

        for (unsigned short i = sm; i < last; ++i) {
            budget::month month = i;

            auto status = budget::compute_month_status(cache, year, month);

            auto savings = status.income - status.expenses;
            auto savings_rate = 0.0;

            if (savings.dollars() > 0) {
                savings_rate = savings / status.income;
            }

            std::string date = "Date.UTC(" + std::to_string(year) + "," + std::to_string(month.value - 1) + ", 1)";

            serie.push_back(savings_rate);
            dates.push_back(date);

            ss << "[" << date << " ," << 100.0 * savings_rate << "],";
        }
    }

    ss << "]},";

    ss << "{ name: '12 months average',";
    ss << "data: [";

    std::array<float, 12> average_12;
    average_12.fill(0.0f);

    for (size_t i = 0; i < serie.size(); ++i) {
        average_12[i % 12] = serie[i];

        auto average = std::accumulate(average_12.begin(), average_12.end(), 0.0f);

        if (i < 12) {
            average = average / float(i + 1);
        } else {
            average = average / 12.f;
        }

        ss << "[" << dates[i] << "," << 100.0 * average << "],";
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);
}

void budget::time_graph_tax_rate_page(html_writer & w) {
    std::stringstream content_stream;

    data_cache cache;

    if (config_contains("taxes_account")) {
       auto taxes_account = config_value("taxes_account");

       if (account_exists(taxes_account)) {
            auto ss = start_time_chart(w, "Tax rate over time", "line", "tax_time_graph", "");

            ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
            ss << R"=====(legend: { enabled: false },)=====";

            ss << "series: [";

            ss << "{ name: 'Tax Rate',";
            ss << "data: [";

            std::vector<float> serie;
            std::vector<std::string> dates;

            auto sy = start_year(cache);

            double max = 1.0;

            for (unsigned short j = sy; j <= budget::local_day().year(); ++j) {
                budget::year year = j;

                auto sm = start_month(cache, year);
                auto last = 13;

                if (year == budget::local_day().year()) {
                    last = budget::local_day().month() + 1;
                }

                for (unsigned short i = sm; i < last; ++i) {
                    budget::month month = i;

                    auto status = budget::compute_month_status(cache, year, month);

                    double tax_rate = status.taxes / status.income;

                    std::string date = "Date.UTC(" + std::to_string(year) + "," + std::to_string(month.value - 1) + ", 1)";

                    serie.push_back(tax_rate);
                    dates.push_back(date);

                    ss << "[" << date << " ," << 100.0 * tax_rate << "],";

                    if (tax_rate > max) {
                        max = tax_rate;
                    }
                }
            }

            ss << "]},";

            ss << "{ name: '12 months average',";
            ss << "data: [";

            std::array<float, 12> average_12;
            average_12.fill(0.0f);

            for (size_t i = 0; i < serie.size(); ++i) {
                average_12[i % 12] = serie[i];

                auto average = std::accumulate(average_12.begin(), average_12.end(), 0.0f);

                if (i < 12) {
                    average = average / float(i + 1);
                } else {
                    average = average / 12.f;
                }

                ss << "[" << dates[i] << "," << 100.0 * average << "],";

                if (average > max) {
                    max = average;
                }
            }

            ss << "]},";

            ss << "]";

            ss << ", yAxis: { min: 0, max: " << (int) (100.0 * max) << ", title: { text: 'Tax Savings Rate' }},";

            end_chart(w, ss);

           return;
       }
    }

    w << "Taxes support not configured";
}

void budget::side_overview_page(html_writer & w, const httplib::Request& req) {
    if(config_value("side_category", "").empty() || config_value("side_prefix", "").empty()) {
        w << "Side hustle is not configured";
    } else {
        if (req.matches.size() == 3) {
            display_side_month_overview(to_number<size_t>(req.matches[2]), to_number<size_t>(req.matches[1]), w);
        } else {
            auto today = budget::local_day();
            display_side_month_overview(today.month(), today.year(), w);
        }
    }
}
