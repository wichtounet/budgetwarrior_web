//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <set>

#include "pages/html_writer.hpp"
#include "pages/net_worth_pages.hpp"
#include "http.hpp"
#include "currency.hpp"
#include "config.hpp"
#include "share.hpp"
#include "views.hpp"

using namespace budget;

void budget::assets_card(budget::html_writer& w){
    w << R"=====(<div class="card">)=====";

    w << R"=====(<div class="card-header card-header-primary">)=====";
    w << R"=====(<div>Assets</div>)=====";
    w << R"=====(</div>)====="; // card-header

    w << R"=====(<div class="card-body">)=====";

    std::string separator = "/";
    if (budget::config_contains("aggregate_separator")) {
        separator = budget::config_value("aggregate_separator");
    }

    // If all assets are in the form group/asset, then we use special style

    bool group_style = !budget::config_contains_and_true("asset_no_group");

    // If one asset has no group, we disable grouping
    if (group_style) {
        for (const auto& asset : w.cache.user_assets()) {
            auto pos = asset.name.find(separator);
            if (pos == 0 || pos == std::string::npos) {
                group_style = false;
                break;
            }
        }
    }

    if (group_style) {
        std::vector<std::string> groups;

        for (const auto& asset : w.cache.user_assets()) {
            std::string group = asset.name.substr(0, asset.name.find(separator));

            if (!range_contains(groups, group)) {
                groups.emplace_back(std::move(group));
            }
        }

        for (auto& group : groups) {
            bool started = false;

            for (const auto& [asset, amount] : w.cache.user_assets() | expand_value(w.cache) | not_zero) {
                if (asset.name.substr(0, asset.name.find(separator)) == group) {
                    auto short_name = asset.name.substr(asset.name.find(separator) + 1);

                    if (!started) {
                        w << "<div class=\"asset_group\">";
                        w << group;
                        w << "</div>";

                        started = true;
                    }

                    w << R"=====(<div class="asset_row row">)=====";
                    w << R"=====(<div class="asset_name col-md-8 col-xl-9 small">)=====";
                    w << short_name;
                    w << R"=====(</div>)=====";
                    w << R"=====(<div class="asset_right col-md-4 col-xl-3 text-right small">)=====";
                    w << R"=====(<span class="asset_amount">)=====";
                    w << budget::to_string(amount) << " " << asset.currency;
                    w << R"=====(</span>)=====";
                    w << R"=====(<br />)=====";
                    w << R"=====(</div>)=====";
                    w << R"=====(</div>)=====";
                }
            }
        }
    } else {
        bool first = true;

        for (const auto& [asset, amount] : w.cache.user_assets() | expand_value_conv(w.cache) | not_zero) {
            if (!first) {
                w << R"=====(<hr />)=====";
            }

            w << R"=====(<div class="row">)=====";
            w << R"=====(<div class="col-md-8 col-xl-9 small">)=====";
            w << asset.name;
            w << R"=====(</div>)=====";
            w << R"=====(<div class="col-md-4 col-xl-3 text-right small">)=====";
            w << budget::to_string(amount) << " " << asset.currency;
            w << R"=====(<br />)=====";
            w << R"=====(</div>)=====";
            w << R"=====(</div>)=====";

            first = false;
        }
    }

    w << R"=====(</div>)====="; //card-body
    w << R"=====(</div>)====="; //card
}

void budget::liabilities_card(budget::html_writer& w){
    if (w.cache.liabilities().empty()) {
        return;
    }

    w << R"=====(<div class="card">)=====";

    w << R"=====(<div class="card-header card-header-primary">)=====";
    w << R"=====(<div>Liabilities</div>)=====";
    w << R"=====(</div>)====="; // card-header

    w << R"=====(<div class="card-body">)=====";

    std::string separator = "/";
    if (budget::config_contains("aggregate_separator")) {
        separator = budget::config_value("aggregate_separator");
    }

    bool first = true;

    for (const auto& [liability, amount] : w.cache.liabilities() | expand_value(w.cache) | not_zero) {
        if (!first) {
            w << R"=====(<hr />)=====";
        }

        w << R"=====(<div class="row">)=====";
        w << R"=====(<div class="col-md-8 col-xl-9 small">)=====";
        w << liability.name;
        w << R"=====(</div>)=====";
        w << R"=====(<div class="col-md-4 col-xl-3 text-right small">)=====";
        w << budget::to_string(amount) << " " << liability.currency;
        w << R"=====(<br />)=====";
        w << R"=====(</div>)=====";
        w << R"=====(</div>)=====";

        first = false;
    }

    w << R"=====(</div>)====="; //card-body
    w << R"=====(</div>)====="; //card
}

void budget::asset_graph_page(html_writer & w, const httplib::Request& req) {
    auto asset = req.matches.size() == 2
        ? get_asset(to_number<size_t>(req.matches[1]))
        : *w.cache.active_user_assets().begin();

    if (req.matches.size() == 2) {
        w << title_begin << "Asset Graph" << budget::active_asset_selector{"assets/graph", to_number<size_t>(req.matches[1])} << title_end;
    } else {
        w << title_begin << "Asset Graph" << budget::active_asset_selector{"assets/graph", 0} << title_end;
    }

    asset_graph(w, "", asset);

    if (asset.currency != get_default_currency()) {
        asset_graph_conv(w, "", asset);
    }

    // Display additional information for share-based assets
    if (asset.share_based) {
        int64_t bought_shares      = 0;
        int64_t last_bought_shares = 0;
        int64_t sold_shares        = 0;
        int64_t current_shares     = 0;

        budget::money buy_price;
        budget::money last_buy_price;
        budget::money sell_price;

        auto current_price  = share_price(asset.ticker);
        date first_date     = local_day();
        bool first_date_set = false;

        for (auto& share : all_asset_shares() | filter_by_asset(asset.id)) {
            if (share.is_buy()) {
                bought_shares += share.shares;
                current_shares += share.shares;
                buy_price += (float) share.shares * share.price;
            }

            if (share.is_sell()) {
                sold_shares += -share.shares;
                current_shares += share.shares;
                sell_price += (float) -share.shares * share.price;
            }

            if (!current_shares) {
                // If the price went down to zero, we need to reset the buying price
                last_bought_shares = bought_shares;
                last_buy_price     = buy_price;

                bought_shares = 0;
                buy_price     = 0;
            }

            if (!first_date_set) {
                first_date     = share.date;
                first_date_set = true;
            }
        }

        if (!current_shares) {
            bought_shares = last_bought_shares;
            buy_price     = last_buy_price;
        }

        w << p_begin << "Number of shares: " << current_shares << p_end;
        w << p_begin << "Current price: " << current_price << p_end;

        if (bought_shares) {
            if (buy_price.positive()) {
                buy_price /= bought_shares;

                w << p_begin << "Average buy price: " << buy_price << p_end;
                w << p_begin << "Invested: " << (float)bought_shares * buy_price << p_end;
                if (current_shares) {
                    w << p_begin << "Value: " << (float)bought_shares * current_price << p_end;
                    w << p_begin << "Current profit: " << (float)bought_shares * (current_price - buy_price) << p_end;
                    w << p_begin << "ROI: " << (100.0f / (buy_price / current_price)) - 100.0f << "%" << p_end;
                }
                w << p_begin << "First Invested: " << budget::to_string(first_date) << p_end;
            } else {
                w << p_begin << "There is an issue with your average buy price! It should be positive" << p_end;
            }
        }

        // TODO This is not correct, since this should use
        // the date of sold and buy to have the correct profit
        // Also, this will not work with several buying and selling periods
        if (sold_shares >= 0) {
            if (sell_price.positive()) {
                sell_price /= sold_shares;

                w << p_begin << p_end;
                w << p_begin << "Sold shares: " << sold_shares << p_end;
                w << p_begin << "Average sold price: " << sell_price << p_end;
                w << p_begin << "Realized profit: " << (float)sold_shares * (sell_price - buy_price) << p_end;
                w << p_begin << "Realized ROI: " << (100.0f / (buy_price / sell_price)) - 100.0f << "%" << p_end;
            } else {
                w << p_begin << "There is an issue with your average sell price! It should be positive" << p_end;
            }
        }
    }
}

void budget::asset_graph(budget::html_writer& w, std::string_view style, const asset& asset) {
    auto ss = start_time_chart(w, asset.name + "(" + asset.currency + ")", "area", "asset_graph", style);

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Net Worth' }},)=====";
    ss << R"=====(legend: { enabled: false },)=====";

    ss << R"=====(subtitle: {)=====";
    ss << "text: '" << get_asset_value(asset, w.cache) << " " << asset.currency << "',";
    ss << R"=====(floating:true, align:"right", verticalAlign: "top", style: { fontWeight: "bold", fontSize: "inherit" })=====";
    ss << R"=====(},)=====";

    ss << "series: [";

    ss << "{ name: 'Value',";
    ss << "data: [";

    auto date     = budget::asset_start_date(w.cache, asset);
    auto end_date = budget::local_day();

    while (date <= end_date) {
        auto sum = get_asset_value(asset, date, w.cache);

        ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::money_to_string(sum) << "],";

        date += days(1);
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);
}

void budget::asset_graph_conv(budget::html_writer& w, std::string_view style, const asset& asset) {
    auto ss = start_time_chart(w, asset.name + "(" + get_default_currency() + ")", "area", "asset_graph_conv", style);

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Net Worth' }},)=====";
    ss << R"=====(legend: { enabled: false },)=====";

    ss << R"=====(subtitle: {)=====";
    ss << "text: '" << get_asset_value_conv(asset, w.cache) << " " << get_default_currency() << "',";
    ss << R"=====(floating:true, align:"right", verticalAlign: "top", style: { fontWeight: "bold", fontSize: "inherit" })=====";
    ss << R"=====(},)=====";

    ss << "series: [";

    ss << "{ name: 'Value',";
    ss << "data: [";

    auto date     = budget::asset_start_date(w.cache, asset);
    auto end_date = budget::local_day();

    while (date <= end_date) {
        auto sum = get_asset_value_conv(asset, date, w.cache);

        ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::money_to_string(sum) << "],";

        date += days(1);
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);
}

namespace {

template <typename Functor>
void net_worth_graph(budget::html_writer& w, std::string_view title, std::string_view style, bool card, Functor nw_func) {
    // if the user does not use assets, this graph does not make sense
    if (no_assets() || no_asset_values()) {
        return;
    }

    auto now               = budget::local_day();
    auto current_net_worth = nw_func(now, w);
    auto y_net_worth       = nw_func({now.year(), 1, 1}, w);
    auto m_net_worth       = nw_func(now - days(now.day() - 1), w);
    auto ytd_growth        = 100.0 * ((1 / (y_net_worth / current_net_worth)) - 1);
    auto mtd_growth        = 100.0 * ((1 / (m_net_worth / current_net_worth)) - 1);

    if (card) {
        w << R"=====(<div class="card">)=====";

        w << R"=====(<div class="card-header card-header-primary">)=====";
        w << R"=====(<div class="float-left">)=====";
        w << title;
        w << R"=====(</div>)=====";
        w << R"=====(<div class="float-right">)=====";
        w << current_net_worth << " __currency__ (YTD: " << ytd_growth << "% MTD: " << mtd_growth << "%)";
        w << R"=====(</div>)=====";
        w << R"=====(<div class="clearfix"></div>)=====";
        w << R"=====(</div>)====="; // card-header

        w << R"=====(<div class="card-body">)=====";
    }

    auto ss = start_time_chart(w, card ? "" : title, "area", "net_worth_graph", style);

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";

    if (current_net_worth.negative()) {
        ss << "yAxis: { title: { text: '" << title << "' }},";
    } else {
        ss << "yAxis: { min: 0, title: { text: '" << title << "' }},";
    }

    ss << R"=====(legend: { enabled: false },)=====";

    if (!card) {
        ss << R"=====(subtitle: {)=====";
        ss << "text: '" << current_net_worth << " __currency__ (YTD: " << ytd_growth << "% MTD: " << mtd_growth << "%)',";
        ss << R"=====(floating:true, align:"right", verticalAlign: "top", style: { fontWeight: "bold", fontSize: "inherit" })=====";
        ss << R"=====(},)=====";
    }

    ss << "series: [";

    ss << "{ name: '" << title << "',";
    ss << "data: [";

    auto date     = budget::asset_start_date(w.cache);
    auto end_date = budget::local_day();

    while (date <= end_date) {
        auto sum = nw_func(date, w);

        ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::money_to_string(sum) << "],";

        date += days(1);
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);

    if (card) {
        w << R"=====(</div>)====="; //card-body
        w << R"=====(</div>)====="; //card
    }
}

} // namespace

void budget::net_worth_graph(budget::html_writer& w, std::string_view style, bool card) {
    ::net_worth_graph(w, "Net Worth", style, card, [](budget::date d, budget::html_writer & w){
        return get_net_worth(d, w.cache);
    });
}

void budget::fi_net_worth_graph(budget::html_writer& w, std::string_view style, bool card) {
    ::net_worth_graph(w, "FI Net Worth", style, card, [](budget::date d, budget::html_writer & w){
        return get_fi_net_worth(d, w.cache);
    });
}

void budget::net_worth_accrual_graph(budget::html_writer& w) {
    // if the user does not use assets, this graph does not make sense
    if (no_assets() || no_asset_values()) {
        return;
    }

    auto ss = start_time_chart(w, "Net worth Accrual", "container", "net_worth_accrual_graph", "");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { title: { text: 'Net Worth Growth' }},)=====";
    ss << R"=====(legend: { enabled: false },)=====";

    ss << "series: [";

    ss << "{ type: 'column', name: 'Net Worth Growth', negativeColor: 'red',";
    ss << "data: [";

    auto date      = budget::asset_start_date(w.cache);
    auto end_date  = budget::local_day();

    // We need to skip the first month
    date += months(1);

    std::vector<budget::money> serie;
    std::vector<std::string> dates;

    while (date <= end_date) {
        budget::money const sum;

        auto start = get_net_worth(date.start_of_month(), w.cache);
        auto end   = get_net_worth(date.end_of_month(), w.cache);

        std::string const date_str =
            "Date.UTC(" + std::to_string(date.year()) + "," + std::to_string(date.month().value - 1) + ", 1)";
        ss << "[" << date_str << " ," << budget::money_to_string(end - start) << "],";

        serie.emplace_back(end - start);
        dates.emplace_back(date_str);

        date += months(1);
    }

    ss << "]},";

    add_average_12_serie(ss, serie, dates);
    add_average_24_serie(ss, serie, dates);

    ss << "]";

    end_chart(w, ss);
}

void budget::net_worth_status_page(html_writer & w) {
    budget::show_asset_values(w);
}

void budget::net_worth_small_status_page(html_writer& w) {
    budget::small_show_asset_values(w);
}

void budget::net_worth_graph_page(html_writer& w) {
    // First, we display the net worth graph
    net_worth_graph(w);

    // Then, we can display some general information

    auto current_net_worth = get_net_worth(w.cache);
    auto now               = budget::local_day();
    auto y_net_worth       = get_net_worth({now.year(), 1, 1}, w.cache);
    auto m_net_worth       = get_net_worth(now - days(now.day() - 1), w.cache);
    auto ytd_growth        = 100.0 * ((1 / (y_net_worth / current_net_worth)) - 1);
    auto mtd_growth        = 100.0 * ((1 / (m_net_worth / current_net_worth)) - 1);

    w << p_begin << "MTD Change " << current_net_worth - m_net_worth << " __currency__" << p_end;
    w << p_begin << "MTD Growth " << mtd_growth << " %" << p_end;

    w << p_begin << "YTD Change " << current_net_worth - y_net_worth << " __currency__" << p_end;
    w << p_begin << "YTD Growth " << ytd_growth << " %" << p_end;

    // Finally, we display the net worth accrual graph
    net_worth_accrual_graph(w);
}

void budget::fi_net_worth_graph_page(html_writer& w) {
    // First, we display the net worth graph
    fi_net_worth_graph(w);

    // Then, we can display some general information

    auto now               = budget::local_day();
    auto current_net_worth = get_fi_net_worth(now, w.cache);
    auto y_net_worth       = get_fi_net_worth({now.year(), 1, 1}, w.cache);
    auto m_net_worth       = get_fi_net_worth(now - days(now.day() - 1), w.cache);
    auto ytd_growth        = 100.0 * ((1 / (y_net_worth / current_net_worth)) - 1);
    auto mtd_growth        = 100.0 * ((1 / (m_net_worth / current_net_worth)) - 1);

    w << p_begin << "MTD Change " << current_net_worth - m_net_worth << " __currency__" << p_end;
    w << p_begin << "MTD Growth " << mtd_growth << " %" << p_end;

    w << p_begin << "YTD Change " << current_net_worth - y_net_worth << " __currency__" << p_end;
    w << p_begin << "YTD Growth " << ytd_growth << " %" << p_end;
}

namespace {

budget::money get_class_sum(data_cache & cache, budget::asset_class & clas, budget::date date) {
    budget::money sum;

    // Add the value of the assets for this class
    for (const auto& asset : cache.user_assets()) {
        sum += get_asset_value_conv(asset, date, cache) * (float(get_asset_class_allocation(asset, clas)) / 100.0f);
    }

    // Remove the value of the liabilities for this class
    for (auto & liability : cache.liabilities()) {
        sum -= get_liability_value_conv(liability, date, cache) * (float(get_asset_class_allocation(liability, clas)) / 100.0f);
    }

    return sum;
}

} // end of anonymous namespace

void budget::net_worth_allocation_page(html_writer& w) {
    // 1. Display the currency breakdown over time

    auto ss = start_time_chart(w, "Net worth allocation", "area", "allocation_time_graph");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Net Worth' }},)=====";
    ss << R"=====(tooltip: {split: true},)=====";
    ss << R"=====(plotOptions: {area: {stacking: 'percent'}},)=====";

    ss << "series: [";

    for (auto & clas : w.cache.asset_classes()) {
        ss << "{ name: '" << clas.name << "',";
        ss << "data: [";

        auto date     = budget::asset_start_date(w.cache);
        auto end_date = budget::local_day();

        while (date <= end_date) {
            auto sum = get_class_sum(w.cache, clas, date);

            ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::money_to_string(sum) << "],";

            date += days(1);
        }

        ss << "]},";
    }

    ss << "]";

    end_chart(w, ss);

    // 2. Display the current currency breakdown

    auto ss2 = start_chart(w, "Current Allocation Breakdown", "pie", "allocation_breakdown_graph");

    ss2 << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    ss2 << "series: [";

    ss2 << "{ name: 'Classes',";
    ss2 << "colorByPoint: true,";
    ss2 << "data: [";

    for (auto & clas : w.cache.asset_classes()) {
        ss2 << "{ name: '" << clas.name << "',";
        ss2 << "y: ";

        auto sum = get_class_sum(w.cache, clas, budget::local_day());
        ss2 << budget::money_to_string(sum);

        ss2 << "},";
    }

    ss2 << "]},";

    ss2 << "]";

    end_chart(w, ss2);
}

void budget::portfolio_allocation_page(html_writer& w) {
    // 1. Display the currency breakdown over time

    auto ss = start_time_chart(w, "Portfolio allocation", "area", "allocation_time_graph");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Net Worth' }},)=====";
    ss << R"=====(tooltip: {split: true},)=====";
    ss << R"=====(plotOptions: {area: {stacking: 'percent'}},)=====";

    ss << "series: [";

    for (auto & clas : w.cache.asset_classes()) {
        ss << "{ name: '" << clas.name << "',";
        ss << "data: [";

        auto date     = budget::asset_start_date(w.cache);
        auto end_date = budget::local_day();

        while (date <= end_date) {
            budget::money sum;

            for (const auto& asset : w.cache.user_assets() | is_portfolio) {
                sum += get_asset_value_conv(asset, date, w.cache) * (float(get_asset_class_allocation(asset, clas)) / 100.0f);
            }

            ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::money_to_string(sum) << "],";

            date += days(1);
        }

        ss << "]},";
    }

    ss << "]";

    end_chart(w, ss);

    // 2. Display the current currency breakdown

    auto ss2 = start_chart(w, "Current Allocation Breakdown", "pie", "allocation_breakdown_graph");

    ss2 << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    ss2 << "series: [";

    ss2 << "{ name: 'Classes',";
    ss2 << "colorByPoint: true,";
    ss2 << "data: [";

    for (auto & clas : w.cache.asset_classes()) {
        ss2 << "{ name: '" << clas.name << "',";
        ss2 << "y: ";

        budget::money sum;

        for (const auto& asset : w.cache.user_assets() | is_portfolio) {
            sum += get_asset_value_conv(asset, w.cache) * (float(get_asset_class_allocation(asset, clas)) / 100.0f);
        }

        ss2 << budget::money_to_string(sum);

        ss2 << "},";
    }

    ss2 << "]},";

    ss2 << "]";

    end_chart(w, ss2);
}

void budget::net_worth_currency_page(html_writer& w) {
    std::set<std::string> currencies;

    for (const auto& asset : w.cache.user_assets()) {
        currencies.insert(asset.currency);
    }

    // 1. Display the currency breakdown over time

    auto ss = start_time_chart(w, "Net worth by currency", "area", "currency_time_graph");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Net Worth' }},)=====";
    ss << R"=====(tooltip: {split: true},)=====";
    ss << R"=====(plotOptions: {area: {stacking: 'percent'}},)=====";

    ss << "series: [";

    for (const auto& currency : currencies) {
        ss << "{ name: '" << currency << "',";
        ss << "data: [";

        auto date     = budget::asset_start_date(w.cache);
        auto end_date = budget::local_day();

        while (date <= end_date) {
            const auto sum = fold_left_auto(w.cache.user_assets() | filter_by_currency(currency) | to_value_conv(w.cache, date))
                             - fold_left_auto(w.cache.liabilities() | filter_by_currency(currency) | to_value_conv(w.cache, date));

            ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::money_to_string(sum) << "],";

            date += days(1);
        }

        ss << "]},";
    }

    ss << "]";

    end_chart(w, ss);

    // 2. Display the value in each currency

    for (const auto& currency : currencies) {
        budget::money net_worth;

        for (const auto& asset : w.cache.user_assets()) {
            net_worth += get_asset_value_conv(asset, currency, w.cache);
        }

        for (auto & liability : w.cache.liabilities()) {
            net_worth -= get_liability_value_conv(liability, currency, w.cache);
        }

        w << p_begin << "Net worth in " << currency << " : " << net_worth << " " << currency << p_end;
    }

    // 3. Display the current currency breakdown

    auto ss2 = start_chart(w, "Current Currency Breakdown", "pie", "currency_breakdown_graph");

    ss2 << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    ss2 << "series: [";

    ss2 << "{ name: 'Currencies',";
    ss2 << "colorByPoint: true,";
    ss2 << "data: [";

    for (const auto& currency : currencies) {
        ss2 << "{ name: '" << currency << "',";
        ss2 << "y: ";

        budget::money sum;

        // Add the assets in this currency
        sum += fold_left_auto(w.cache.user_assets() | filter_by_currency(currency) | to_value_conv(w.cache));

        // Remove the liabilities in this currency
        sum -= fold_left_auto(w.cache.liabilities() | filter_by_currency(currency) | to_value_conv(w.cache));

        ss2 << budget::money_to_string(sum);

        ss2 << "},";
    }

    ss2 << "]},";

    ss2 << "]";

    end_chart(w, ss2);
}

void budget::portfolio_status_page(html_writer& w) {
    budget::show_asset_portfolio(w);

    make_tables_sortable(w);
}

void budget::portfolio_currency_page(html_writer& w) {
    std::set<std::string> currencies;

    for (const auto& asset : w.cache.user_assets()) {
        if (asset.portfolio) {
            currencies.insert(asset.currency);
        }
    }

    // 1. Display the currency breakdown over time

    auto ss = start_time_chart(w, "Portfolio by currency", "area", "portfolio_currency_graph");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Sum' }},)=====";
    ss << R"=====(tooltip: {split: true},)=====";
    ss << R"=====(plotOptions: {area: {stacking: 'percent'}},)=====";

    ss << "series: [";

    for (const auto& currency : currencies) {
        ss << "{ name: '" << currency << "',";
        ss << "data: [";

        auto date     = budget::asset_start_date(w.cache);
        auto end_date = budget::local_day();

        while (date <= end_date) {
            const auto sum = fold_left_auto(w.cache.user_assets() | filter_by_currency(currency) | is_portfolio | to_value_conv(w.cache, date));

            ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::money_to_string(sum) << "],";

            date += days(1);
        }

        ss << "]},";
    }

    ss << "]";

    end_chart(w, ss);

    // 2. Display the current currency breakdown

    auto ss2 = start_chart(w, "Current Currency Breakdown", "pie", "currency_breakdown_graph");

    ss2 << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    ss2 << "series: [";

    ss2 << "{ name: 'Currencies',";
    ss2 << "colorByPoint: true,";
    ss2 << "data: [";

    for (const auto& currency : currencies) {
        ss2 << "{ name: '" << currency << "',";
        ss2 << "y: ";

        const auto sum = fold_left_auto(w.cache.user_assets() | filter_by_currency(currency) | is_portfolio | to_value_conv(w.cache));

        ss2 << budget::money_to_string(sum);

        ss2 << "},";
    }

    ss2 << "]},";

    ss2 << "]";

    end_chart(w, ss2);
}

void budget::portfolio_graph_page(html_writer& w) {
    auto ss = start_time_chart(w, "Portfolio", "area");

    ss << R"=====(xAxis: { type: 'datetime', title: { text: 'Date' }},)=====";
    ss << R"=====(yAxis: { min: 0, title: { text: 'Portfolio' }},)=====";

    ss << R"=====(subtitle: {)=====";
    ss << "text: '" << get_portfolio_value() << " __currency__',";
    ss << R"=====(floating:true, align:"right", verticalAlign: "top", style: { fontWeight: "bold", fontSize: "inherit" })=====";
    ss << R"=====(},)=====";

    ss << "series: [";

    ss << "{ name: 'Portfolio',";
    ss << "data: [";

    auto date     = budget::asset_start_date(w.cache);
    auto end_date = budget::local_day();

    while (date <= end_date) {
        const auto sum = fold_left_auto(w.cache.user_assets() | is_portfolio | to_value_conv(w.cache, date));

        ss << "[Date.UTC(" << date.year() << "," << date.month().value - 1 << "," << date.day() << ") ," << budget::money_to_string(sum) << "],";

        date += days(1);
    }

    ss << "]},";

    ss << "]";

    end_chart(w, ss);
}

void rebalance_page_base(html_writer& w, bool nocash) {
    // 1. Display the rebalance table

    budget::show_asset_rebalance(w, nocash);

    make_tables_sortable(w);

    w << R"=====(<div class="row">)=====";

    // 2. Display the current allocation

    w << R"=====(<div class="col-lg-6 col-md-12">)=====";

    // Collect the amounts per asset

    std::map<size_t, budget::money> asset_amounts;

    for (const auto& asset : w.cache.user_assets() | is_portfolio) {
        if (nocash && asset.is_cash()) {
            continue;
        }

        asset_amounts[asset.id] = get_asset_value(asset, w.cache);
    }

    // Compute the colors for each asset that will be displayed

    std::map<size_t, size_t> colors;

    for (const auto& asset : w.cache.user_assets()) {
        if (nocash && asset.is_cash()) {
            continue;
        }

        if (asset.portfolio && (asset_amounts[asset.id] || asset.portfolio_alloc)) {
            if (!colors.contains(asset.id)) {
                auto c           = colors.size();
                colors[asset.id] = c;
            }
        }
    }

    // Compute the colors for the first graph

    std::stringstream current_ss;

    current_ss << R"=====(var current_base_colors = ["#7cb5ec", "#434348", "#90ed7d", "#f7a35c", "#8085e9", "#f15c80", "#e4d354", "#2b908f", "#f45b5b", "#91e8e1", "red", "blue", "green"];)=====";

    current_ss << "var current_pie_colors = (function () {";
    current_ss << "var colors = [];";

    for (auto& [asset_id, amount] : asset_amounts) {
        if (amount) {
            current_ss << "colors.push(current_base_colors[" << colors[asset_id] << "]);";
        }
    }

    current_ss << "return colors;";
    current_ss << "}());";

    auto ss = start_chart(w, "Current Allocation", "pie", "current_allocation_graph");

    ss << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    ss << "series: [";

    ss << "{ name: 'Assets',";
    ss << "colorByPoint: true,";
    ss << "colors: current_pie_colors,";
    ss << "data: [";

    budget::money sum;

    for (auto& [asset_id, amount] : asset_amounts) {
        if (amount) {
            auto asset       = get_asset(asset_id);
            auto conv_amount = amount * exchange_rate(asset.currency);

            ss << "{ name: '" << asset.name << "',";
            ss << "y: ";
            ss << budget::money_to_string(conv_amount);
            ss << "},";

            sum += conv_amount;
        }
    }

    ss << "]},";

    ss << "]";

    current_ss << ss.str();

    end_chart(w, current_ss);

    w << R"=====(</div>)=====";

    // 3. Display the desired allocation

    // Compute the colors for the second graph

    std::stringstream desired_ss;

    desired_ss << R"=====(var desired_base_colors = ["#7cb5ec", "#434348", "#90ed7d", "#f7a35c", "#8085e9", "#f15c80", "#e4d354", "#2b908f", "#f45b5b", "#91e8e1", "red", "blue", "green"];)=====";

    desired_ss << "var desired_pie_colors = (function () {";
    desired_ss << "var colors = [];";

    for (const auto& asset : w.cache.user_assets()) {
        if (asset.portfolio && asset.portfolio_alloc) {
            desired_ss << "colors.push(desired_base_colors[" << colors[asset.id] << "]);";
        }
    }

    desired_ss << "return colors;";
    desired_ss << "}());";

    w << R"=====(<div class="col-lg-6 col-md-12">)=====";

    auto ss2 = start_chart(w, "Desired Allocation", "pie", "desired_allocation_graph");

    ss2 << R"=====(tooltip: { pointFormat: '<b>{point.y} __currency__ ({point.percentage:.1f}%)</b>' },)=====";

    ss2 << "series: [";

    ss2 << "{ name: 'Assets',";
    ss2 << "colorByPoint: true,";
    ss2 << "colors: desired_pie_colors,";
    ss2 << "data: [";

    for (const auto& asset : w.cache.user_assets()) {
        if (asset.portfolio && asset.portfolio_alloc) {
            ss2 << "{ name: '" << asset.name << "',";
            ss2 << "y: ";
            ss2 << budget::money_to_string(sum * (static_cast<float>(asset.portfolio_alloc) / 100.0f));
            ss2 << "},";
        }
    }

    ss2 << "]},";

    ss2 << "]";

    desired_ss << ss2.str();

    end_chart(w, desired_ss);

    w << R"=====(</div>)=====";

    w << R"=====(</div>)=====";
}

void budget::rebalance_page(html_writer& w) {
    rebalance_page_base(w, false);
}

void budget::rebalance_nocash_page(html_writer& w) {
    rebalance_page_base(w, true);
}
