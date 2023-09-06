//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "pages/index_pages.hpp"
#include "incomes.hpp"
#include "pages/earnings_pages.hpp"
#include "pages/expenses_pages.hpp"
#include "pages/objectives_pages.hpp"
#include "pages/net_worth_pages.hpp"
#include "pages/html_writer.hpp"
#include "http.hpp"
#include "config.hpp"
#include "views.hpp"

using namespace budget;

namespace {

budget::money monthly_income(data_cache & cache, budget::month month, budget::year year) {
    // TODO: This only work if monthly_income is called today
    return get_base_income(cache) + fold_left_auto(cache.earnings() | filter_by_date(year, month) | to_amount);
}

budget::money monthly_spending(data_cache & cache, budget::month month, budget::year year) {
    return fold_left_auto(cache.expenses() | filter_by_date(year, month) | to_amount);
}

void cash_flow_card(budget::html_writer& w){
    const auto today = budget::local_day();

    const auto m = today.month();
    const auto y = today.year();

    w << R"=====(<div class="card">)=====";

    auto income = monthly_income(w.cache, m, y);
    auto spending = monthly_spending(w.cache, m, y);

    w << R"=====(<div class="card-header card-header-primary">)=====";
    w << R"=====(<div class="float-left">Cash Flow</div>)=====";
    w << R"=====(<div class="float-right">)=====";
    w << income - spending << " __currency__";

    if(income > spending){
        w << " (" << 100.0f * ((income - spending) / income) << "%)";
    }

    w << R"=====(</div>)=====";
    w << R"=====(<div class="clearfix"></div>)=====";
    w << R"=====(</div>)====="; // card-header

    w << R"=====(<div class="row card-body">)=====";

    w << R"=====(<div class="col-md-6 col-sm-12">)=====";
    month_breakdown_income_graph(w, "Income", m, y, true, "min-width:300px; height: 300px;");
    w << R"=====(</div>)====="; //column

    w << R"=====(<div class="col-md-6 col-sm-12">)=====";
    month_breakdown_expenses_graph(w, "Expenses", m, y, true, "min-width:300px; height: 300px;");
    w << R"=====(</div>)====="; //column

    w << R"=====(</div>)====="; //card-body
    w << R"=====(</div>)====="; //card
}

} // namespace

void budget::index_page(html_writer & w) {
    const bool left_column = !no_assets() && !no_asset_values();

    if (left_column) {
        // A. The left column

        w << R"=====(<div class="row">)=====";

        w << R"=====(<div class="col-lg-4 d-none d-lg-block">)====="; // left column

        assets_card(w);

        liabilities_card(w);

        w << R"=====(</div>)====="; // left column

        // B. The right column

        w << R"=====(<div class="col-lg-8 col-md-12">)====="; // right column
    }

    // 1. Display the net worth graph
    net_worth_graph(w, "min-width: 300px; width: 100%; height: 300px;", true);

    // 2. Cash flow
    cash_flow_card(w);

    // 3. Display the objectives status
    objectives_card(w);

    if (left_column) {
        w << R"=====(</div>)====="; // right column

        w << R"=====(</div>)====="; // row
    }
}
