//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>
#include <numeric>

#include <openssl/md5.h>

#include "cpp_utils/string.hpp"

#include "config.hpp"
#include "overview.hpp"
#include "summary.hpp"
#include "version.hpp"
#include "pages/html_writer.hpp"
#include "currency.hpp"
#include "budget_exception.hpp"
#include "logging.hpp"

// Include all the pages
#include "pages/assets_pages.hpp"
#include "pages/asset_values_pages.hpp"
#include "pages/asset_shares_pages.hpp"
#include "pages/asset_classes_pages.hpp"
#include "pages/liabilities_pages.hpp"
#include "pages/fortunes_pages.hpp"
#include "pages/wishes_pages.hpp"
#include "pages/index_pages.hpp"
#include "pages/report_pages.hpp"
#include "pages/debts_pages.hpp"
#include "pages/accounts_pages.hpp"
#include "pages/incomes_pages.hpp"
#include "pages/expenses_pages.hpp"
#include "pages/earnings_pages.hpp"
#include "pages/retirement_pages.hpp"
#include "pages/recurrings_pages.hpp"
#include "pages/objectives_pages.hpp"
#include "pages/overview_pages.hpp"
#include "pages/net_worth_pages.hpp"
#include "pages/user_pages.hpp"
#include "pages/web_config.hpp"
#include "http.hpp"
#include "views.hpp"

using namespace budget;

namespace {

static constexpr const char new_line = '\n';

std::string header(const std::string& title, bool menu = true) {
    std::stringstream stream;

    // The header

    stream << R"=====(
        <!doctype html>
        <html lang="en">
          <head>
            <meta charset="utf-8">
            <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">

            <meta name="description" content="budgetwarrior">
            <meta name="author" content="Baptiste Wicht">

            <!-- The CSS -->

            <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/4.0.0-beta.3/css/bootstrap.min.css" integrity="sha256-PCsx7lOyGhyGmzsO5MGXhzwV6UpNTlNf1p6V6w2CppQ=" crossorigin="anonymous" />

            <style>
                body {
                  padding-top: 5rem;
                }

                p {
                    margin-bottom: 8px;
                }

                .asset_group {
                    margin-left: -20px;
                    margin-right: -20px;
                    padding-left: 5px;
                    border-bottom: 1px solid #343a40;
                    font-weight: bold;
                    color: #343a40;
                }

                .asset_row {
                    padding-top: 3px;
                }

                .asset_row:not(:last-child) {
                    border-bottom: 1px solid rgba(0,0,0,0.125);
                }

                .asset_name {
                    font-weight: bold;
                    color: #007bff;
                    padding-left: 5px;
                }

                .asset_right {
                    padding-left: 0px;
                    padding-right: 5px;
                }

                .asset_date {
                    color: rgba(0,0,0,0.5);
                }

                .small-form-inline {
                    float: left;
                    padding-right: 10px;
                }

                .small-text {
                    font-size: 10pt;
                }

                .extend-only {
                    width: 75%;
                }

                .selector a {
                    font-size: xx-large;
                }

                .selector select {
                    vertical-align: middle;
                    margin-bottom: 22px;
                    margin-left: 2px;
                    margin-right: 2px;
                }

                .card {
                    margin-bottom: 10px !important;
                }

                .card-header-primary {
                    color:white !important;
                    background-color: #007bff !important;
                    padding: 0.5rem 0.75rem !important;
                }

                .gauge-cash-flow-title {
                    margin-top: -15px;
                }

                .gauge-objective-title {
                    color: rgb(124, 181, 236);
                    margin-top: -15px;
                    text-align: center;
                }

                .default-graph-style {
                    min-width: 300px;
                    height: 400px;
                    margin: 0 auto;
                }

                .dataTables_wrapper {
                    padding-left: 0px !important;
                    padding-right: 0px !important;
                }

                .flat-hr {
                    margin:0px;
                }

                input[type=radio] {
                    margin-left: 10px;
                }
            </style>
    )=====";

    if (title.empty()) {
        stream << "<title>budgetwarrior</title>";
    } else {
        stream << "<title>budgetwarrior - " << title << "</title>";
    }

    stream << new_line;

    stream << "</head>" << new_line;
    stream << "<body>" << new_line;

    // The navigation

    stream << R"=====(<nav class="navbar navbar-expand-md navbar-dark bg-dark fixed-top">)=====";

    stream << "<a class=\"navbar-brand\" href=\"#\">" << budget::get_version() << "</a>";

    if (menu) {
        stream << R"=====(
          <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarsExampleDefault" aria-controls="navbarsExampleDefault" aria-expanded="false" aria-label="Toggle navigation">
            <span class="navbar-toggler-icon"></span>
          </button>
          <div class="collapse navbar-collapse" id="navbarsExampleDefault">
            <ul class="navbar-nav mr-auto">
              <li class="nav-item">
                <a class="nav-link" href="/">Index <span class="sr-only">(current)</span></a>
              </li>
        )=====";

        // Overview

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown01" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Overview</a>
                <div class="dropdown-menu" aria-labelledby="dropdown01">
                  <a class="dropdown-item" href="/overview/">Overview Month</a>
                  <a class="dropdown-item" href="/overview/year/">Overview Year</a>
                  <a class="dropdown-item" href="/overview/aggregate/year/">Aggregate Year</a>
                  <a class="dropdown-item" href="/overview/aggregate/year_fv/">Aggregate Year FV</a>
                  <a class="dropdown-item" href="/overview/aggregate/year_month/">Aggregate Year per month</a>
                  <a class="dropdown-item" href="/overview/aggregate/month/">Aggregate Month</a>
                  <a class="dropdown-item" href="/overview/aggregate/all/">Aggregate All</a>
        )=====";

        if (is_side_hustle_enabled()) {
            stream << R"=====(
                  <a class="dropdown-item" href="/side_hustle/overview/">Side Hustle Overview Month</a>
            )=====";
        }

        stream << R"=====(
                  <a class="dropdown-item" href="/report/">Report</a>
                  <a class="dropdown-item" href="/overview/savings/time/">Savings rate over time</a>
                  <a class="dropdown-item" href="/overview/tax/time/">Tax rate over time</a>
                </div>
              </li>
        )=====";

        // Assets

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown02" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Assets</a>
                <div class="dropdown-menu" aria-labelledby="dropdown02">
                  <a class="dropdown-item" href="/assets/">Assets</a>
                  <a class="dropdown-item" href="/net_worth/status/">Net worth Status</a>
                  <a class="dropdown-item" href="/net_worth/graph/">Net worth Graph</a>
                  <a class="dropdown-item" href="/net_worth/allocation/">Net worth Allocation</a>
                  <a class="dropdown-item" href="/net_worth/currency/">Net worth Currency</a>
                  <a class="dropdown-item" href="/portfolio/status/">Portfolio Status</a>
                  <a class="dropdown-item" href="/portfolio/graph/">Portfolio Graph</a>
                  <a class="dropdown-item" href="/portfolio/allocation/">Portfolio Allocation</a>
                  <a class="dropdown-item" href="/portfolio/currency/">Portfolio Currency</a>
                  <a class="dropdown-item" href="/rebalance/">Rebalance</a>
                  <a class="dropdown-item" href="/assets/add/">Add Asset</a>
                  <a class="dropdown-item" href="/assets/graph/">Asset Graph</a>
                  <a class="dropdown-item" href="/asset_values/list/">Asset Values</a>
                  <a class="dropdown-item" href="/asset_values/batch/full/">Full Batch Update</a>
                  <a class="dropdown-item" href="/asset_values/batch/current/">Current Batch Update</a>
                  <a class="dropdown-item" href="/asset_values/add/">Set One Asset Value</a>
                  <a class="dropdown-item" href="/asset_shares/list/">Asset Shares</a>
                  <a class="dropdown-item" href="/asset_shares/add/">Add Asset Share</a>
                  <div class="dropdown-divider"></div>
                  <a class="dropdown-item" href="/liabilities/">Liabilities</a>
                  <a class="dropdown-item" href="/liabilities/add/">Add Liability</a>
                  <a class="dropdown-item" href="/asset_values/add/liability/">Set One Liability Value</a>
                  <div class="dropdown-divider"></div>
                  <a class="dropdown-item" href="/asset_classes/list/">Asset Classes</a>
                  <a class="dropdown-item" href="/asset_classes/add/">Add Asset Class</a>
                </div>
              </li>
        )=====";

        // Expenses

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown03" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Expenses</a>
                <div class="dropdown-menu" aria-labelledby="dropdown03">
                  <a class="dropdown-item" href="/expenses/add/">Add Expense</a>
                  <a class="dropdown-item" href="/expenses/">Expenses</a>
                  <a class="dropdown-item" href="/expenses/search/">Search</a>
                  <a class="dropdown-item" href="/expenses/all/">All Expenses</a>
                  <a class="dropdown-item" href="/expenses/breakdown/month/">Expenses Breakdown Month</a>
                  <a class="dropdown-item" href="/expenses/breakdown/year/">Expenses Breakdown Year</a>
                  <a class="dropdown-item" href="/expenses/time/">Expenses over time</a>
                </div>
              </li>
        )=====";

        // Earnings

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown04" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Earnings</a>
                <div class="dropdown-menu" aria-labelledby="dropdown04">
                  <a class="dropdown-item" href="/earnings/add/">Add Earning</a>
                  <a class="dropdown-item" href="/earnings/">Earnings</a>
                  <a class="dropdown-item" href="/earnings/search/">Search</a>
                  <a class="dropdown-item" href="/earnings/all/">All Earnings</a>
                  <a class="dropdown-item" href="/earnings/time/">Earnings over time</a>
                  <a class="dropdown-item" href="/income/time/">Income over time</a>
                </div>
              </li>
        )=====";

        // Accounts

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown05" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Accounts</a>
                <div class="dropdown-menu" aria-labelledby="dropdown05">
                  <a class="dropdown-item" href="/accounts/">Accounts</a>
                  <a class="dropdown-item" href="/accounts/all/">All Accounts</a>
                  <a class="dropdown-item" href="/accounts/add/">Add Account</a>
                  <a class="dropdown-item" href="/accounts/archive/month/">Archive Account (month)</a>
                  <a class="dropdown-item" href="/accounts/archive/year/">Archive Account (year)</a>
                  <div class="dropdown-divider"></div>
                  <a class="dropdown-item" href="/incomes/">Incomes</a>
                  <a class="dropdown-item" href="/incomes/set/">Set Income</a>
                </div>
              </li>
        )=====";

        // Retirement

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown_retirement" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Retirement</a>
                <div class="dropdown-menu" aria-labelledby="dropdown_retirement">
                  <a class="dropdown-item" href="/retirement/status/">Status</a>
                  <a class="dropdown-item" href="/retirement/configure/">Configure</a>
                  <a class="dropdown-item" href="/retirement/fi/">FI Ratio Over Time</a>
                  <a class="dropdown-item" href="/retirement/net_worth/">FI Net Worth</a>
                </div>
              </li>
        )=====";

        // Fortune

        if(!budget::is_fortune_disabled()){
            stream << R"=====(
                  <li class="nav-item dropdown">
                    <a class="nav-link dropdown-toggle" href="#" id="dropdown06" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Fortune</a>
                    <div class="dropdown-menu" aria-labelledby="dropdown06">
                      <a class="dropdown-item" href="/fortunes/graph/">Fortune</a>
                      <a class="dropdown-item" href="/fortunes/status/">Status</a>
                      <a class="dropdown-item" href="/fortunes/list/">List</a>
                      <a class="dropdown-item" href="/fortunes/add/">Set fortune</a>
                    </div>
                  </li>
            )=====";
        }

        // Goals

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown07" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Goals</a>
                <div class="dropdown-menu" aria-labelledby="dropdown07">
                  <a class="dropdown-item" href="/objectives/status/">Status</a>
                  <a class="dropdown-item" href="/objectives/list/">List</a>
                  <a class="dropdown-item" href="/objectives/add/">Add Goal</a>
                </div>
              </li>
        )=====";

        // Wishes

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown08" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Wishes</a>
                <div class="dropdown-menu" aria-labelledby="dropdown08">
                  <a class="dropdown-item" href="/wishes/status/">Status</a>
                  <a class="dropdown-item" href="/wishes/list/">List</a>
                  <a class="dropdown-item" href="/wishes/estimate/">Estimate</a>
                  <a class="dropdown-item" href="/wishes/add/">Add Wish</a>
                </div>
              </li>
        )=====";

        // Others

        stream << R"=====(
              <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="dropdown_others" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Others</a>
                <div class="dropdown-menu" aria-labelledby="dropdown_others">
                  <a class="dropdown-item" href="/user/config/">Configuration</a>
                  <div class="dropdown-divider"></div>
                  <a class="dropdown-item" href="/recurrings/list/">List Recurring Operations</a>
                  <a class="dropdown-item" href="/recurrings/add/">Add Recurring Operation</a>
        )=====";

        if (!budget::is_debts_disabled()) {
            stream << R"=====(
                      <div class="dropdown-divider"></div>
                      <a class="dropdown-item" href="/debts/add/">Add Debt</a>
                      <a class="dropdown-item" href="/debts/list/">List Debts</a>
                      <a class="dropdown-item" href="/debts/all/">All Debts</a>
                    </div>
                  </li>
            )=====";
        }

        stream << R"=====(
                </div>
              </li>
        )=====";

        // Finish the menu
        stream << R"=====(
            </ul>
          </div>
        )=====";
    }

    stream << "</nav>" << new_line;

    // The main component

    stream << R"=====(<main class="container-fluid">)=====" << new_line;
    //stream << "<div>" << new_line;

    return stream.str();
}

void display_message(budget::writer& w, const httplib::Request& req) {
    if (req.has_param("message")) {
        if (req.has_param("error")) {
            w << R"=====(<div class="alert alert-danger" role="alert">)=====";
        } else if (req.has_param("success")) {
            w << R"=====(<div class="alert alert-success" role="alert">)=====";
        } else {
            w << R"=====(<div class="alert alert-primary" role="alert">)=====";
        }

        w << req.get_param_value("message");
        w << R"=====(</div>)=====";
    }
}

void replace_all(std::string& source, std::string_view from, std::string_view to) {
    size_t current_pos = 0;

    while ((current_pos = source.find(from, current_pos)) != std::string::npos) {
        source.replace(current_pos, from.length(), to);
        current_pos += to.length();
    }
}

void filter_html(std::string& html, const httplib::Request& req) {
    if (req.has_param("input_name")) {
        replace_all(html, "__budget_this_page__", html_base64_encode(req.path + "?input_name=" + req.get_param_value("input_name")));
    } else {
        replace_all(html, "__budget_this_page__", html_base64_encode(req.path));
    }

    replace_all(html, "__currency__", get_default_currency());
}

//Note: This must be synchronized with page_end
std::string footer() {
    return "</main></body></html>";
}

bool parameters_present(const httplib::Request& req, std::vector<const char*> parameters) {
    return !std::ranges::any_of(parameters, [&req] (const auto & param) { return req.has_param(param); } );
}

void call_render_function(html_writer&            w,
                          const httplib::Request& req,
                          httplib::Response&      res,
                          void (*render_function)(html_writer&, const httplib::Request&)) {
    cpp_unused(res);
    render_function(w, req);
}

void call_render_function(html_writer&            w,
                          const httplib::Request& req,
                          httplib::Response&      res,
                          void (*render_function)(html_writer&)) {
    cpp_unused(req);
    cpp_unused(res);
    render_function(w);
}

template <typename T>
auto render_wrapper(const char* title, T render_function) {
    return [title, render_function](const httplib::Request& req, httplib::Response& res) {
        std::stringstream content_stream;

        budget::html_writer w(content_stream);

        try {
            if (!page_start(req, res, content_stream, title)) {
                return;
            }

            call_render_function(w, req, res, render_function);

            page_end(w, req, res);
        } catch (const budget_exception& e) {
            display_error_message(w, "Exception occured: " + e.message());
            LOG_F(ERROR, "budget_exception occured in render({}): {}", req.path, e.message());
            page_end(w, req, res);
        } catch (const date_exception& e) {
            display_error_message(w, "Exception occured: " + e.message());
            LOG_F(ERROR, "date_exception occured in render({}): {}", req.path, e.message());
            page_end(w, req, res);
        } catch (...) {
            display_error_message(w, "Unknown Exception occured");
            LOG_F(FATAL, "unknown_exception occured in render({}): {}", req.path);
            page_end(w, req, res);
        }
    };
}

} //end of anonymous namespace

void budget::load_pages(httplib::Server& server) {
    // Declare all the pages
    server.Get("/", render_wrapper("", &index_page));

    server.Get("/overview/year/", render_wrapper("Yearly Overview", &overview_year_page));
    server.Get(R"(/overview/year/(\d+)/)", render_wrapper("Yearly Overview", &overview_year_page));
    server.Get("/overview/", render_wrapper("Monthly Overview", &overview_page));
    server.Get(R"(/overview/(\d+)/(\d+)/)", render_wrapper("Monthly Overview",&overview_page));
    server.Get("/overview/aggregate/year/", render_wrapper("Yearly Aggregate", &overview_aggregate_year_page));
    server.Get(R"(/overview/aggregate/year/(\d+)/)", render_wrapper("Yearly Aggregate", &overview_aggregate_year_page));
    server.Get("/overview/aggregate/year_month/", render_wrapper("Yearly Aggregate", &overview_aggregate_year_month_page));
    server.Get(R"(/overview/aggregate/year_month/(\d+)/)", render_wrapper("Yearly Aggregate", &overview_aggregate_year_month_page));
    server.Get("/overview/aggregate/year_fv/", render_wrapper("Yearly Aggregate", &overview_aggregate_year_fv_page));
    server.Get(R"(/overview/aggregate/year_fv/(\d+)/)", render_wrapper("Yearly Aggregate", &overview_aggregate_year_fv_page));
    server.Get("/overview/aggregate/month/", render_wrapper("Monthly Aggregate", &overview_aggregate_month_page));
    server.Get(R"(/overview/aggregate/month/(\d+)/(\d+)/)", render_wrapper("Monthly Aggregate", &overview_aggregate_month_page));
    server.Get("/overview/aggregate/all/", render_wrapper("Aggregate", &overview_aggregate_all_page));

    server.Get("/overview/savings/time/", render_wrapper("Savings Rate Over Time", &time_graph_savings_rate_page));
    server.Get("/overview/tax/time/", render_wrapper("Tax Rate Over Time", &time_graph_tax_rate_page));

    server.Get("/side_hustle/overview/", render_wrapper("Side Hustle Overview", &side_overview_page));
    server.Get(R"(/side_hustle/overview/(\d+)/(\d+)/)", render_wrapper("Side Hustle Overview", &side_overview_page));

    server.Get("/report/", render_wrapper("Report", &report_page));

    server.Get("/accounts/", render_wrapper("Accounts", &accounts_page));
    server.Get("/accounts/all/", render_wrapper("Accounts", &all_accounts_page));
    server.Get("/accounts/add/", render_wrapper("Accounts", &add_accounts_page));
    server.Get("/accounts/edit/", render_wrapper("Accounts", &edit_accounts_page));
    server.Get("/accounts/archive/month/", render_wrapper("Accounts", &archive_accounts_month_page));
    server.Get("/accounts/archive/year/", render_wrapper("Accounts", &archive_accounts_year_page));

    server.Get("/incomes/", render_wrapper("Incomes", &incomes_page));
    server.Get("/incomes/set/", render_wrapper("Incomes", &set_incomes_page));
    server.Get("/income/time/", render_wrapper("Income over time", &time_graph_income_page));

    server.Get(R"(/expenses/(\d+)/(\d+)/)", render_wrapper("Expenses", &expenses_page));
    server.Get("/expenses/", render_wrapper("Expenses", &expenses_page));
    server.Get("/expenses/search/", render_wrapper("Expenses", &search_expenses_page));

    server.Get(R"(/expenses/breakdown/month/(\d+)/(\d+)/)", render_wrapper("Expenses Breakdown", &month_breakdown_expenses_page));
    server.Get("/expenses/breakdown/month/", render_wrapper("Expenses Breakdown", &month_breakdown_expenses_page));
    server.Get(R"(/expenses/breakdown/year/(\d+)/)", render_wrapper("Expenses Breakdown", &year_breakdown_expenses_page));
    server.Get("/expenses/breakdown/year/", render_wrapper("Expenses Breakdown", &year_breakdown_expenses_page));

    server.Get("/expenses/time/", render_wrapper("Expenses", &time_graph_expenses_page));
    server.Get("/expenses/all/", render_wrapper("Expenses", &all_expenses_page));
    server.Get("/expenses/add/", render_wrapper("Expenses", &add_expenses_page));
    server.Get("/expenses/edit/", render_wrapper("Expenses", &edit_expenses_page));

    server.Get(R"(/earnings/(\d+)/(\d+)/)", render_wrapper("Earnings", &earnings_page));
    server.Get("/earnings/", render_wrapper("Earnings", &earnings_page));
    server.Get("/earnings/search/", render_wrapper("Earnings", &search_earnings_page));
    server.Get("/earnings/time/", render_wrapper("Earnings", &time_graph_earnings_page));
    server.Get("/earnings/all/", render_wrapper("Earnings", &all_earnings_page));
    server.Get("/earnings/add/", render_wrapper("Earnings", &add_earnings_page));
    server.Get("/earnings/edit/", render_wrapper("Earnings", &edit_earnings_page));

    server.Get("/portfolio/status/", render_wrapper("Portfolio", &portfolio_status_page));
    server.Get("/portfolio/graph/", render_wrapper("Portfolio", &portfolio_graph_page));
    server.Get("/portfolio/currency/", render_wrapper("Portfolio", &portfolio_currency_page));
    server.Get("/portfolio/allocation/", render_wrapper("Portfolio", &portfolio_allocation_page));
    server.Get("/rebalance/", render_wrapper("Rebalance", &rebalance_page));
    server.Get("/rebalance/nocash/", render_wrapper("Rebalance", &rebalance_nocash_page));
    server.Get("/assets/", render_wrapper("Assets", &assets_page));
    server.Get("/net_worth/status/", render_wrapper("Net Worth", &net_worth_status_page));
    server.Get("/net_worth/status/small/", render_wrapper("Net Worth", &net_worth_small_status_page)); // Not in the menu for now
    server.Get("/net_worth/graph/", render_wrapper("Net Worth", &net_worth_graph_page));
    server.Get("/net_worth/currency/", render_wrapper("Net Worth", &net_worth_currency_page));
    server.Get("/net_worth/allocation/", render_wrapper("Net Worth", &net_worth_allocation_page));
    server.Get("/assets/add/", render_wrapper("Assets", &add_assets_page));
    server.Get("/assets/edit/", render_wrapper("Assets", &edit_assets_page));
    server.Get(R"(/assets/graph/(\d+)/)", render_wrapper("Assets", &asset_graph_page));
    server.Get("/assets/graph/", render_wrapper("Assets", &asset_graph_page));

    server.Get("/asset_values/list/", render_wrapper("Asset Values", &list_asset_values_page));
    server.Get("/asset_values/add/", render_wrapper("Asset Values", &add_asset_values_page));
    server.Get("/asset_values/add/liability/", render_wrapper("Asset Values", &add_asset_values_liability_page));
    server.Get("/asset_values/batch/full/", render_wrapper("Asset Values", &full_batch_asset_values_page));
    server.Get("/asset_values/batch/current/", render_wrapper("Asset Values", &current_batch_asset_values_page));
    server.Get("/asset_values/edit/", render_wrapper("Asset Values", &edit_asset_values_page));

    server.Get("/asset_shares/list/", render_wrapper("Asset Shares", &list_asset_shares_page));
    server.Get("/asset_shares/add/", render_wrapper("Asset Shares", &add_asset_shares_page));
    server.Get("/asset_shares/edit/", render_wrapper("Asset Shares", &edit_asset_shares_page));

    server.Get("/asset_classes/list/", render_wrapper("Asset Classes", &list_asset_classes_page));
    server.Get("/asset_classes/add/", render_wrapper("Asset Classes", &add_asset_classes_page));
    server.Get("/asset_classes/edit/", render_wrapper("Asset Classes", &edit_asset_classes_page));

    server.Get("/liabilities/", render_wrapper("Liabilities", &list_liabilities_page));
    server.Get("/liabilities/list/", render_wrapper("Liabilities", &list_liabilities_page));
    server.Get("/liabilities/add/", render_wrapper("Liabilities", &add_liabilities_page));
    server.Get("/liabilities/edit/", render_wrapper("Liabilities", &edit_liabilities_page));

    server.Get("/objectives/list/", render_wrapper("Ojbectives", &list_objectives_page));
    server.Get("/objectives/status/", render_wrapper("Ojbectives", &status_objectives_page));
    server.Get("/objectives/add/", render_wrapper("Ojbectives", &add_objectives_page));
    server.Get("/objectives/edit/", render_wrapper("Ojbectives", &edit_objectives_page));

    server.Get("/wishes/list/", render_wrapper("Wishes", &wishes_list_page));
    server.Get("/wishes/status/", render_wrapper("Wishes", &wishes_status_page));
    server.Get("/wishes/estimate/", render_wrapper("Wishes", &wishes_estimate_page));
    server.Get("/wishes/add/", render_wrapper("Wishes", &add_wishes_page));
    server.Get("/wishes/edit/", render_wrapper("Wishes", &edit_wishes_page));

    server.Get("/retirement/status/", render_wrapper("Retirement", &retirement_status_page));
    server.Get("/retirement/configure/", render_wrapper("Retirement", &retirement_configure_page));
    server.Get("/retirement/fi/", render_wrapper("Retirement", &retirement_fi_ratio_over_time));
    server.Get("/retirement/net_worth/", render_wrapper("FI Net Worth", &fi_net_worth_graph_page));

    server.Get("/recurrings/list/", render_wrapper("Recurring Operations", &recurrings_list_page));
    server.Get("/recurrings/add/", render_wrapper("Recurring Operations", &add_recurrings_page));
    server.Get("/recurrings/edit/", render_wrapper("Recurring Operations", &edit_recurrings_page));

    server.Get("/debts/list/", render_wrapper("Debts", &budget::list_debts_page));
    server.Get("/debts/all/", render_wrapper("Debts", &budget::all_debts_page));
    server.Get("/debts/add/", render_wrapper("Debts", &budget::add_debts_page));
    server.Get("/debts/edit/", render_wrapper("Debts", &budget::edit_debts_page));

    server.Get("/fortunes/graph/", render_wrapper("Fortunes", &graph_fortunes_page));
    server.Get("/fortunes/status/", render_wrapper("Fortunes", &status_fortunes_page));
    server.Get("/fortunes/list/", render_wrapper("Fortunes", &list_fortunes_page));
    server.Get("/fortunes/add/", render_wrapper("Fortunes", &add_fortunes_page));
    server.Get("/fortunes/edit/", render_wrapper("Fortunes", &edit_fortunes_page));

    server.Get("/user/config/", render_wrapper("Configuration", &user_config_page));

    // Handle error

    server.set_error_handler([](const auto& req, auto& res) {
        std::stringstream content_stream;
        content_stream.imbue(std::locale("C"));

        if (res.status == 401 || res.status == 403) {
            content_stream << header("", false);
        } else {
            content_stream << header("", true);
        }

        content_stream << "<p>Error Status: <span class='text-danger'>";
        content_stream << res.status;
        content_stream << "</span></p>";

        content_stream << "<p>On Page: <span class='text-success'>";
        content_stream << req.path;
        content_stream << "</span></p>";

        content_stream << footer();

        res.set_content(content_stream.str(), "text/html");
    });
}

namespace {

std::string md5_to_string(unsigned char * h) {
    std::stringstream ss;

    ss << std::hex << std::setfill('0');

    for (size_t i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        long c = h[i];
        ss << std::setw(2) << c;
    }

    return ss.str();
}

std::string md5_direct(const std::string & base) {
    unsigned char hash[MD5_DIGEST_LENGTH];

    MD5((unsigned char*) base.c_str(), base.size(), hash);

    return md5_to_string(hash);
}

void ask_for_digest(httplib::Response& res) {
    // The opaque value
    std::string opaque = "budgetwarrior";

    // Generate the random nonce
    std::random_device rd;
    std::mt19937_64 g(rd());
    std::string nonce = std::to_string(g());

    auto nonce_str  = "nonce=\"" + md5_direct(opaque) + "\"";
    auto opaque_str = "opaque=\"" + md5_direct(nonce) + "\"";

    res.status = 401;
    res.set_header("WWW-Authenticate", "Digest realm=\"budgetwarrior\", qop=\"auth,auth-int\"," + nonce_str + "," + opaque_str);
}

} // end of anonymous namespace

bool budget::authenticate(const httplib::Request& req, httplib::Response& res) {
    if (is_secure()) {
        if (req.has_header("Authorization")) {
            auto authorization = req.get_header_value("Authorization");

            if (authorization.substr(0, 7) != "Digest ") {
                ask_for_digest(res);

                LOG_F(INFO, "Unauthorized Access: Not digest realm ({})", req.path);

                return false;
            }

            // Extract the part after Digest
            auto sub_authorization = authorization.substr(7, authorization.size());

            std::map<std::string, std::string> dict;
            auto parts = split(sub_authorization, ',');

            for (auto & part : parts) {
                // Each part is supposed to be key=value
                // Some of the values are in quotes

                cpp::trim(part);
                auto mid_pos = part.find('=');

                if (mid_pos == std::string::npos) {
                    continue;
                }

                auto key       = part.substr(0, mid_pos);
                auto value_raw = part.substr(mid_pos + 1);

                std::string value;
                if (value_raw.size() >= 2 && value_raw[0] == '\"' && value_raw.back() == '\"') {
                    value = value_raw.substr(1, value_raw.size() - 2);
                } else {
                    value = value_raw;
                }

                dict[key] = value;
            }

            if (dict["username"].empty() || dict["nonce"].empty() || dict["response"].empty() || dict["opaque"].empty() || dict["realm"].empty()
                || dict["nc"].empty()) {
                ask_for_digest(res);

                LOG_F(INFO, "Unauthorized Access: Missing some digest credentials ({})", req.path);

                return false;
            }

            auto username = dict["username"];

            if (username != get_web_user()) {
                ask_for_digest(res);

                LOG_F(WARNING, "Unauthorized Access: Invalid username {} ({})", username, req.path);

                return false;
            }

            // At this stage, we have to compute the nonce, like the client, and
            // compare to what the client answered

            std::string response_final = md5_direct(md5_direct(get_web_user() + ":" + dict["realm"] + ":" + get_web_password())
                                                    + ":" + dict["nonce"] + ":" + dict["nc"] + ":" + dict["cnonce"] + ":" + dict["qop"] + ":"
                                                    + md5_direct(req.method + ":" + dict["uri"]));

            if (dict["response"] != response_final) {
                ask_for_digest(res);

                LOG_F(WARNING, "Unauthorized Access: Invalid response for {} ({})", username, req.path);

                return false;
            }

            LOG_F(INFO, "Valid authentication for {} ({})", username, req.path);

            return true;
        } else {
            ask_for_digest(res);

            LOG_F(WARNING, "Unauthorized Access: No authentication ({})", req.path, req.path);

            return false;
        }
    }

    return true;
}

bool budget::page_start(const httplib::Request& req, httplib::Response& res, std::stringstream& content_stream, const std::string& title) {
    content_stream.imbue(std::locale("C"));

    if (!authenticate(req, res)) {
        return false;
    }

    content_stream << header(title);

    budget::html_writer w(content_stream);
    display_message(w, req);

    return true;
}

bool budget::validate_parameters(html_writer& w, const httplib::Request& req, std::vector<const char*> parameters) {
    if(!parameters_present(req, parameters)){
        display_error_message(w, "Invalid parameter for the request");

        return false;
    }

    return true;
}

void budget::page_end(budget::html_writer & w, const httplib::Request& req, httplib::Response& res) {
    w << "</main>";
    w.load_deferred_scripts();
    w << "</body></html>";

    auto result = w.os.str();

    filter_html(result, req);

    res.set_content(result, "text/html");
}

void budget::make_tables_sortable(budget::html_writer& w){
    w.defer_script(R"=====(
        $(".table").DataTable({
         "columnDefs": [ {
          "targets": 'not-sortable',
          "orderable": false,
         }]
        });
    )=====");

    w.use_module("datatables");
}

void budget::display_error_message(budget::writer& w, const std::string& message) {
    w << R"=====(<div class="alert alert-danger" role="alert">)=====";
    w << message;
    w << R"=====(</div>)=====";
}

void budget::form_begin(budget::writer& w, const std::string& action, const std::string& back_page) {
    w << R"=====(<form method="POST" action=")=====";
    w << action;
    w << R"=====(">)=====";
    w << R"=====(<input type="hidden" name="server" value="yes">)=====";
    w << R"=====(<input type="hidden" name="back_page" value=")=====";
    w << html_base64_encode(back_page);
    w << R"=====(">)=====";
}

void budget::page_form_begin(budget::writer& w, const std::string& action) {
    w << R"=====(<form method="GET" action=")=====";
    w << action;
    w << R"=====(">)=====";
}

void budget::form_begin_edit(budget::writer& w, const std::string& action, const std::string& back_page, const std::string& input_id) {
    form_begin(w, action, html_base64_decode(back_page));

    w << R"=====(<input type="hidden" name="input_id" value=")=====";
    w << input_id;
    w << R"=====(">)=====";
}

void budget::form_end(budget::writer& w, const std::string& button) {
    if (button.empty()) {
        w << R"=====(<button type="submit" class="btn btn-primary">Submit</button>)=====";
    } else {
        w << R"=====(<button type="submit" class="btn btn-primary">)=====";
        w << button;
        w << R"=====(</button>)=====";
    }

    w << "</form>";
}

void budget::add_text_picker(budget::writer& w, const std::string& title, const std::string& name, const std::string& default_value, bool required) {
    w << R"=====(<div class="form-group">)=====";

    w << "<label for=\"" << name << "\">" << title << "</label>";

    if (required) {
        w << "<input required type=\"text\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    } else {
        w << "<input type=\"text\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    }

    if (default_value.empty()) {
        w << " placeholder=\"Enter " << title << "\"";
    } else {
        w << " value=\"" << default_value << "\" ";
    }

    w << R"=====(
            >
         </div>
    )=====";
}

void budget::add_password_picker(budget::writer& w, const std::string& title, const std::string& name, const std::string& default_value, bool required) {
    w << R"=====(<div class="form-group">)=====";

    w << "<label for=\"" << name << "\">" << title << "</label>";

    if (required) {
        w << "<input required type=\"password\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    } else {
        w << "<input type=\"password\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    }

    if (default_value.empty()) {
        w << " placeholder=\"Enter " << title << "\"";
    } else {
        w << " value=\"" << default_value << "\" ";
    }

    w << R"=====(
            >
         </div>
    )=====";
}


void budget::add_name_picker(budget::writer& w, const std::string& default_value) {
    add_text_picker(w, "Name", "input_name", default_value);
}

void budget::add_title_picker(budget::writer& w, const std::string& default_value) {
    add_text_picker(w, "Title", "input_title", default_value);
}

void budget::add_amount_picker(budget::writer& w, const std::string& default_value) {
    add_money_picker(w, "amount", "input_amount", default_value);
}

void budget::add_paid_amount_picker(budget::writer& w, const std::string& default_value) {
    add_money_picker(w, "paid amount", "input_paid_amount", default_value);
}

void budget::add_yes_no_picker(budget::writer& w, const std::string& title, const std::string& name, bool default_value) {
    w << R"=====(<div class="form-group">)=====";

    w << "<label for=\"" << name << "\">" << title << "</label>";

    if (default_value) {
        w << R"=====(<label class="radio-inline"><input type="radio" name=")=====";
        w << name;
        w << R"=====(" value="yes" checked>Yes</label>)=====";
    } else {
        w << R"=====(<label class="radio-inline"><input type="radio" name=")=====";
        w << name;
        w << R"=====(" value="yes">Yes</label>)=====";
    }

    if (!default_value) {
        w << R"=====(<label class="radio-inline"><input type="radio" name=")=====";
        w << name;
        w << R"=====(" value="no" checked>No</label>)=====";
    } else {
        w << R"=====(<label class="radio-inline"><input type="radio" name=")=====";
        w << name;
        w << R"=====(" value="no">No</label>)=====";
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void budget::add_paid_picker(budget::writer& w, bool paid) {
    add_yes_no_picker(w, "Paid", "input_paid", paid);
}

void budget::add_date_picker(budget::writer& w, const std::string& default_value, bool one_line) {
    if (one_line) {
        w << R"=====(<div class="form-group row">)=====";

        w << "<label class=\"col-sm-4 col-form-label\" for=\"input_date\">Date</label>";

        w << R"=====(<div class="col-sm-4">)=====";
    } else {
        w << R"=====(<div class="form-group">)=====";

        w << "<label for=\"input_date\">Date</label>";
    }

    auto today = budget::local_day();

    w << R"=====(<input required type="date" class="form-control" id="input_date" name="input_date" value=")=====";

    if (default_value.empty()) {
        w << today.year() << "-";

        if (today.month() < 10) {
            w << "0" << today.month().value << "-";
        } else {
            w << today.month().value << "-";
        }

        if (today.day() < 10) {
            w << "0" << today.day();
        } else {
            w << today.day();
        }
    } else {
        w << default_value;
    }

    w << "\">";

    if (one_line) {
        w << "</div>";
        w << "</div>";
    } else {
        w << "</div>";
    }
}

std::stringstream budget::start_chart_base(budget::html_writer& w, const std::string& chart_type, const std::string& id, std::string style) {
    w.use_module("highcharts");

    w << R"=====(<div id=")=====";
    w << id;

    if (style.empty()) {
        w << R"=====(" class="default-graph-style"></div>)=====" << end_of_line;
    } else {
        w << R"=====(" style="margin: 0 auto; )=====";
        w << style;
        w << R"=====("></div>)=====" << end_of_line;
    }

    std::stringstream ss;
    ss.imbue(std::locale("C"));

    ss << R"=====(Highcharts.chart(')=====";
    ss << id;
    ss << R"=====(', {)=====";

    ss << R"=====(chart: {type: ')=====";
    ss << chart_type;
    ss << R"=====('},)=====";

    ss << R"=====(credits: { enabled: false },)=====";
    ss << R"=====(exporting: { enabled: false },)=====";

    return ss;
}

std::stringstream budget::start_chart(budget::html_writer& w, const std::string& title, const std::string& chart_type,
                                      const std::string& id, std::string style) {
    auto ss = start_chart_base(w, chart_type, id, style);

    ss << R"=====(title: {text: ')=====";
    ss << title;
    ss << R"=====('},)=====";

    return ss;
}

std::stringstream budget::start_time_chart(budget::html_writer& w, const std::string& title, const std::string& chart_type,
                                           const std::string& id, std::string style) {
    // Note: Not nice but we are simply injecting zoomType here
    auto ss = start_chart_base(w, chart_type + "', zoomType: 'x", id, style);

    ss << R"=====(title: {text: ')=====";
    ss << title;
    ss << R"=====('},)=====";

    ss << R"=====(rangeSelector: {enabled: true}, )=====";

    return ss;
}

void budget::end_chart(budget::html_writer& w, std::stringstream& ss) {
    ss << R"=====(});)=====";

    w.defer_script(ss.str());
}

void budget::add_average_12_serie(std::stringstream& ss,
                                 std::vector<budget::money> serie,
                                 std::vector<std::string> dates) {
    ss << "{ type: 'line', name: '12 months average',";
    ss << "data: [";

    std::array<budget::money, 12> average_12;

    for (size_t i = 0; i < serie.size(); ++i) {
        average_12[i % 12] = serie[i];

        auto average = std::accumulate(average_12.begin(), average_12.end(), budget::money());

        if (i < 12) {
            average = average / int(i + 1);
        } else {
            average = average / 12;
        }

        ss << "[" << dates[i] << "," << budget::money_to_string(average) << "],";
    }

    ss << "]},";
}

void budget::add_average_5_serie(std::stringstream& ss,
                                 std::vector<budget::money> serie,
                                 std::vector<std::string> dates) {
    ss << "{ name: '5 year average',";
    ss << "data: [";

    std::array<budget::money, 5> average_5;

    for (size_t i = 0; i < serie.size(); ++i) {
        average_5[i % 5] = serie[i];

        auto average = std::accumulate(average_5.begin(), average_5.end(), budget::money());

        if (i < 5) {
            average = average / int(i + 1);
        } else {
            average = average / 5;
        }

        ss << "[" << dates[i] << "," << budget::money_to_string(average) << "],";
    }

    ss << "]},";
}

void budget::add_account_picker(budget::writer& w, budget::date day, const std::string& default_value) {
    w << R"=====(
            <div class="form-group">
                <label for="input_account">Account</label>
                <select class="form-control" id="input_account" name="input_account">
    )=====";

    for (auto& account : all_accounts(w.cache, day.year(), day.month())) {
        if (budget::to_string(account.id) == default_value) {
            w << "<option selected value=\"" << account.id << "\">" << account.name << "</option>";
        } else {
            w << "<option value=\"" << account.id << "\">" << account.name << "</option>";
        }
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void budget::add_account_picker_by_name(
        budget::writer& w, budget::date day, const std::string& title, const std::string& default_value, const std::string& input, bool allow_empty) {
    w << "<div class=\"form-group\"><label for=\"" << input << "\">" << title << "</label>";
    w << "<select class=\"form-control\" id=\"" << input << "\" name=\"" << input << "\">";

    if (allow_empty) {
        if ("" == default_value) {
            w << "<option selected value=\"\"></option>";
        } else {
            w << "<option value=\"\"></option>";
        }
    }

    for (auto& account : all_accounts(w.cache, day.year(), day.month())) {
        if (account.name == default_value) {
            w << "<option selected value=\"" << account.name << "\">" << account.name << "</option>";
        } else {
            w << "<option value=\"" << account.name << "\">" << account.name << "</option>";
        }
    }

    w << R"=====(</select></div>)=====";
}

void budget::add_share_asset_picker(budget::writer& w, const std::string& default_value) {
    w << R"=====(
            <div class="form-group">
                <label for="input_asset">Asset</label>
                <select class="form-control" id="input_asset" name="input_asset">
    )=====";

    for (auto& asset : w.cache.user_assets() | share_based_only) {
        if (budget::to_string(asset.id) == default_value) {
            w << "<option selected value=\"" << asset.id << "\">" << asset.name << "</option>";
        } else {
            w << "<option value=\"" << asset.id << "\">" << asset.name << "</option>";
        }
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void budget::add_value_asset_picker(budget::writer& w, const std::string& default_value) {
    w << R"=====(
            <div class="form-group">
                <label for="input_asset">Asset</label>
                <select class="form-control" id="input_asset" name="input_asset">
    )=====";

    for (auto& asset : w.cache.user_assets() | not_share_based) {
        if (budget::to_string(asset.id) == default_value) {
            w << "<option selected value=\"" << asset.id << "\">" << asset.name << "</option>";
        } else {
            w << "<option value=\"" << asset.id << "\">" << asset.name << "</option>";
        }
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void budget::add_active_share_asset_picker(budget::writer& w, const std::string& default_value) {
    w << R"=====(
            <div class="form-group">
                <label for="input_asset">Asset</label>
                <select class="form-control" id="input_asset" name="input_asset">
    )=====";

    for (auto& asset : w.cache.active_user_assets() | share_based_only) {
        if (budget::to_string(asset.id) == default_value) {
            w << "<option selected value=\"" << asset.id << "\">" << asset.name << "</option>";
        } else {
            w << "<option value=\"" << asset.id << "\">" << asset.name << "</option>";
        }
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void budget::add_active_value_asset_picker(budget::writer& w, const std::string& default_value) {
    w << R"=====(
            <div class="form-group">
                <label for="input_asset">Asset</label>
                <select class="form-control" id="input_asset" name="input_asset">
    )=====";

    for (auto& asset : w.cache.active_user_assets() | not_share_based) {
        if (budget::to_string(asset.id) == default_value) {
            w << "<option selected value=\"" << asset.id << "\">" << asset.name << "</option>";
        } else {
            w << "<option value=\"" << asset.id << "\">" << asset.name << "</option>";
        }
    }

    w << R"=====(
                </select>
            </div>
    )=====";
}

void budget::add_liability_picker(budget::writer& w, const std::string& default_value) {
    w << R"=====(
            <div class="form-group">
                <label for="input_asset">Liability</label>
                <select class="form-control" id="input_asset" name="input_asset">
    )=====";

    for (auto& liability : all_liabilities()) {
        if (budget::to_string(liability.id) == default_value) {
            w << "<option selected value=\"" << liability.id << "\">" << liability.name << "</option>";
        } else {
            w << "<option value=\"" << liability.id << "\">" << liability.name << "</option>";
        }
    }

    w << R"=====(
                </select>
            </div>
            <input type="hidden" name="input_liability" value="true" />
    )=====";
}

void budget::add_integer_picker(budget::writer& w, const std::string& title, const std::string& name, bool negative, const std::string& default_value) {
    w << R"=====(<div class="form-group">)=====";

    w << "<label for=\"" << name << "\">" << title << "</label>";

    if (negative) {
        w << "<input required type=\"number\" step=\"1\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    } else {
        w << "<input required type=\"number\" min=\"0\" step=\"1\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    }

    if (default_value.empty()) {
        w << " placeholder=\"Enter " << title << "\" ";
    } else {
        w << " value=\"" << default_value << "\" ";
    }

    w << ">";

    w << "</div>";
}

void budget::add_money_picker(budget::writer& w, const std::string& title, const std::string& name, const std::string& default_value, bool required,
                              bool one_line, const std::string& currency) {
    if(!currency.empty() && !one_line){
        throw budget_exception("add_money_picker currency only works with one_line", true);
    }

    if (one_line) {
        w << R"=====(<div class="form-group row">)=====";

        w << "<label class=\"col-sm-4 col-form-label\" for=\"" << name << "\">" << title << "</label>";

        w << R"=====(<div class="col-sm-4">)=====";
    } else {
        w << R"=====(<div class="form-group">)=====";

        w << "<label for=\"" << name << "\">" << title << "</label>";
    }

    if (required) {
        w << "<input required type=\"number\" step=\"0.01\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    } else {
        w << "<input type=\"number\" step=\"0.01\" class=\"form-control\" id=\"" << name << "\" name=\"" << name << "\" ";
    }

    if (default_value.empty()) {
        w << " placeholder=\"Enter " << title << "\" ";
    } else {
        w << " value=\"" << default_value << "\" ";
    }

    w << ">";

    if (one_line) {
        w << "</div>";

        if (!currency.empty()) {
            w << "<label class=\"col-sm-2 col-form-label\">" << currency << "</label>";
        }

        w << "</div>";
    } else {
        w << "</div>";
    }
}
