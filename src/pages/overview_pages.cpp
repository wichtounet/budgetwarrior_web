//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <numeric>

#include "overview.hpp"

#include "pages/html_writer.hpp"
#include "pages/overview_pages.hpp"
#include "pages/web_config.hpp"
#include "http.hpp"
#include "config.hpp"
#include "compute.hpp"

using namespace budget;

void budget::overview_page(html_writer& w, const httplib::Request& req) {
    if (req.matches.size() == 3) {
        display_month_overview(month_from_string(req.matches[2]), year_from_string(req.matches[1]), w);
    } else {
        auto today = budget::local_day();
        display_month_overview(today.month(), today.year(), w);
    }
}

void budget::overview_aggregate_all_page(html_writer& w) {
    // Configuration of the overview
    const bool        full           = config_contains_and_true("aggregate_full");
    const bool        disable_groups = config_contains_and_true("aggregate_no_group");
    std::string const separator      = config_value("aggregate_separator", "/");

    aggregate_all_overview(w, full, disable_groups, separator);
}

void budget::overview_aggregate_year_page(html_writer& w, const httplib::Request& req) {
    // Configuration of the overview
    const bool        full           = config_contains_and_true("aggregate_full");
    const bool        disable_groups = config_contains_and_true("aggregate_no_group");
    std::string const separator      = config_value("aggregate_separator", "/");

    if (req.matches.size() == 2) {
        aggregate_year_overview(w, full, disable_groups, separator, year_from_string(req.matches[1]));
    } else {
        auto today = budget::local_day();
        aggregate_year_overview(w, full, disable_groups, separator, today.year());
    }
}

void budget::overview_aggregate_year_fv_page(html_writer& w, const httplib::Request& req) {
    // Configuration of the overview
    const bool        full           = config_contains_and_true("aggregate_full");
    const bool        disable_groups = config_contains_and_true("aggregate_no_group");
    std::string const separator      = config_value("aggregate_separator", "/");

    if (req.matches.size() == 2) {
        aggregate_year_fv_overview(w, full, disable_groups, separator, year_from_string(req.matches[1]));
    } else {
        auto today = budget::local_day();
        aggregate_year_fv_overview(w, full, disable_groups, separator, today.year());
    }
}

void budget::overview_aggregate_year_month_page(html_writer& w, const httplib::Request& req) {
    // Configuration of the overview
    const bool        full           = config_contains_and_true("aggregate_full");
    const bool        disable_groups = config_contains_and_true("aggregate_no_group");
    std::string const separator      = config_value("aggregate_separator", "/");

    if (req.matches.size() == 2) {
        aggregate_year_month_overview(w, full, disable_groups, separator, year_from_string(req.matches[1]));
    } else {
        auto today = budget::local_day();
        aggregate_year_month_overview(w, full, disable_groups, separator, today.year());
    }
}

void budget::overview_aggregate_month_page(html_writer& w, const httplib::Request& req) {
    // Configuration of the overview
    const bool        full           = config_contains_and_true("aggregate_full");
    const bool        disable_groups = config_contains_and_true("aggregate_no_group");
    std::string const separator      = config_value("aggregate_separator", "/");

    if (req.matches.size() == 3) {
        aggregate_month_overview(w, full, disable_groups, separator, month_from_string(req.matches[2]), year_from_string(req.matches[1]));
    } else {
        auto today = budget::local_day();
        aggregate_month_overview(w, full, disable_groups, separator, today.month(), today.year());
    }
}

void budget::overview_year_page(html_writer& w, const httplib::Request& req) {
    budget::year year = budget::local_day().year();
    if (req.matches.size() == 2) {
        year = year_from_string(req.matches[1]);
    }

    // Display the Summary Yearly Overview
    display_year_overview_header(year, w);

    const auto last = last_month(year);

    // Display the Yearly expense

    {
        auto ss = start_time_chart(w, "Expenses", "line", "year_overview_expenses_time_graph", "");

        ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
        ss << R"=====(yAxis: { min: 0, title: { text: 'Monthly Expenses' }},)=====";
        ss << R"=====(legend: { enabled: false },)=====";

        ss << "series: [";

        ss << "{ name: '" << year << " Expenses',";
        ss << "data: [";

        for (budget::month month = start_month(w.cache, year); month < last; ++month) {
            auto sum = fold_left_auto(all_expenses_month(w.cache, year, month) | to_amount);

            const std::string date = std::format("Date.UTC({},{},1)", year.value, month.value - 1);
            ss << "[" << date << "," << budget::money_to_string(sum) << "],";
        }

        ss << "]},";

        if (year - date_type(1) >= start_year(w.cache)) {
            ss << "{ name: '" << year - date_type(1) << " Expenses',";
            ss << "data: [";

            for (budget::month month = start_month(w.cache, year - date_type(1)); month.is_valid(); ++month) {
                auto sum = fold_left_auto(all_expenses_month(w.cache, year - date_type(1), month) | to_amount);

                const std::string date = std::format("Date.UTC({},{},1)", year.value, month.value - date_type(1));
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

        for (budget::month month = start_month(w.cache, year); month < last; ++month) {
            auto sum = get_base_income(w.cache, budget::date(year, month, 2)) + fold_left_auto(all_earnings_month(w.cache, year, month) | to_amount);

            const std::string date = std::format("Date.UTC({},{},1)", year.value, month.value - 1);
            ss << "[" << date << "," << budget::money_to_string(sum) << "],";
        }

        ss << "]},";

        if (year - date_type(1) >= start_year(w.cache)) {
            ss << "{ name: '" << year - date_type(1) << " Expenses',";
            ss << "data: [";

            for (budget::month month = start_month(w.cache, year - date_type(1)); month.is_valid(); ++month) {
                auto sum =
                        get_base_income(w.cache, budget::date(year - date_type(1), month, 2)) + fold_left_auto(all_earnings_month(w.cache, year - date_type(1), month) | to_amount);

                const std::string date = std::format("Date.UTC({},{},1)", year.value, month.value - date_type(1));
                ss << "[" << date << "," << budget::money_to_string(sum) << "],";
            }

            ss << "]},";
        }

        ss << "]";

        end_chart(w, ss);
    }

    // Display The Full Table Yearly Overview
    display_year_overview(year, w);
}

void budget::time_graph_savings_rate_page(html_writer& w) {
    auto ss = start_time_chart(w, "Savings rate over time", "line", "savings_time_graph", "");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, max: 100, title: { text: 'Monthly Savings Rate' }},)=====";
    ss << R"=====(legend: { enabled: false },)=====";

    ss << "series: [";

    ss << "{ name: 'Savings Rate',";
    ss << "data: [";

    std::vector<float>       serie;
    std::vector<std::string> dates;

    auto sy = start_year(w.cache);

    for (budget::year year = sy; year <= budget::local_day().year(); ++year) {
        const auto sm   = start_month(w.cache, year);
        const auto last = last_month(year);

        for (budget::month month = sm; month < last; ++month) {
            auto status = budget::compute_month_status(w.cache, year, month);

            auto savings      = status.income - status.expenses;
            float savings_rate = 0.0;

            if (savings.dollars() > 0) {
                savings_rate = savings / status.income;
            }

            const std::string date = std::format("Date.UTC({},{},1)", year.value, month.value - 1);

            serie.push_back(savings_rate);
            dates.push_back(date);

            ss << "[" << date << " ," << 100.0 * savings_rate << "],";
        }
    }

    ss << "]},";

    ss << "{ name: '12 months average',";
    ss << "data: [";

    std::array<float, 12> average_12{};
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

void budget::time_graph_tax_rate_page(html_writer& w) {
    std::stringstream const content_stream;

    if (has_taxes_account()) {
        auto ss = start_time_chart(w, "Tax rate over time", "line", "tax_time_graph", "");

        ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
        ss << R"=====(legend: { enabled: false },)=====";

        ss << "series: [";

        ss << "{ name: 'Tax Rate',";
        ss << "data: [";

        std::vector<float>       serie;
        std::vector<std::string> dates;

        auto sy = start_year(w.cache);

        double max = 1.0;

        for (budget::year year = sy; year <= budget::local_day().year(); ++year) {
            const auto sm   = start_month(w.cache, year);
            const auto last = last_month(year);

            for (budget::month month = sm; month < last; ++month) {
                auto status = budget::compute_month_status(w.cache, year, month);

                float tax_rate = status.taxes / status.income;

                const std::string date = std::format("Date.UTC({},{},1)", year.value, month.value - 1);

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

        std::array<float, 12> average_12{};
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

    w << "Taxes support not configured";
}

namespace {

void display_side_month_overview(budget::month month, budget::year year, budget::writer& writer) {
    auto accounts = all_accounts(writer.cache, year, month);

    writer << title_begin << "Side Hustle Overview of " << month << " " << year << budget::year_month_selector{"side_hustle/overview", year, month}
           << title_end;

    auto side_category = user_config_value("side_category", "");
    auto side_prefix   = user_config_value("side_prefix", "");

    std::vector<std::vector<std::string>> contents;
    std::vector<money>                    total_expenses(1, budget::money());
    std::vector<money>                    total_earnings(1, budget::money());

    std::vector<std::string>     columns = {side_category};
    cpp::string_hash_map<size_t> indexes = {{side_category, 0}};

    std::vector<budget::expense> side_expenses;
    std::vector<budget::earning> side_earnings;

    for (const auto& expense : writer.cache.expenses()) {
        if (get_account(expense.account).name == side_category) {
            if (side_prefix.empty() || expense.name.find(side_prefix) == 0) {
                side_expenses.push_back(expense);
            }
        }
    }

    for (const auto& earning : writer.cache.earnings()) {
        if (get_account(earning.account).name == side_category) {
            if (side_prefix.empty() || earning.name.find(side_prefix) == 0) {
                side_earnings.push_back(earning);
            }
        }
    }

    // Expenses
    add_expenses_column(month, year, "Expenses", contents, indexes, columns.size(), side_expenses, total_expenses);

    // Earnings
    contents.emplace_back(columns.size() * 3, "");
    add_earnings_column(month, year, "Earnings", contents, indexes, columns.size(), side_earnings, total_earnings);

    writer.display_table(columns, contents, 3);

    auto income             = total_earnings[0];
    auto total_all_expenses = total_expenses[0];

    budget::money const savings      = income - total_all_expenses;
    double              savings_rate = 0.0;

    if (savings.value > 0) {
        savings_rate = 100 * (savings / income);
    }

    std::vector<std::string>              second_columns;
    std::vector<std::vector<std::string>> second_contents;

    second_contents.emplace_back(std::vector<std::string>{"Total expenses", budget::to_string(total_all_expenses)});
    second_contents.emplace_back(std::vector<std::string>{"Total earnings", budget::to_string(income)});
    second_contents.emplace_back(std::vector<std::string>{"Savings", budget::to_string(savings)});
    second_contents.emplace_back(std::vector<std::string>{"Savings Rate", budget::to_string(savings_rate) + "%"});

    writer.display_table(second_columns, second_contents, 1, {}, accounts.size() * 9 + 1);
}

} // end of anonymous namespace

void budget::side_overview_page(html_writer& w, const httplib::Request& req) {
    if (!budget::is_side_hustle_enabled()) {
        w << "Side hustle is not configured";
        return;
    }

    if (req.matches.size() == 3) {
        display_side_month_overview(month_from_string(req.matches[2]), year_from_string(req.matches[1]), w);
    } else {
        auto today = budget::local_day();
        display_side_month_overview(today.month(), today.year(), w);
    }
}
