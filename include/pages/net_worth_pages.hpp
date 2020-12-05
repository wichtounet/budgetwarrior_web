//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pages/server_pages.hpp"

namespace budget {

struct asset;

// Net Worth Pages
void net_worth_status_page(html_writer& w);
void net_worth_small_status_page(html_writer& w);
void net_worth_graph_page(html_writer& w);
void net_worth_allocation_page(html_writer& w);
void portfolio_allocation_page(html_writer& w);
void net_worth_currency_page(html_writer& w);
void portfolio_status_page(html_writer& w);
void portfolio_currency_page(html_writer& w);
void portfolio_graph_page(html_writer& w);
void rebalance_page(html_writer& w);
void rebalance_nocash_page(html_writer& w);
void asset_graph_page(html_writer& w, const httplib::Request& req);

// Net Worth utilities
void assets_card(budget::html_writer& w);
void liabilities_card(budget::html_writer& w);
void net_worth_graph(budget::html_writer& w, const std::string style = "", bool card = false);
void asset_graph(budget::html_writer& w, const std::string style, const asset& asset);

} //end of namespace budget
