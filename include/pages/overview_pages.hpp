//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "server_pages.hpp"

namespace budget {

void overview_page(html_writer& w, const httplib::Request& req);
void overview_aggregate_all_page(html_writer& w);
void overview_aggregate_year_page(html_writer& w, const httplib::Request& req);
void overview_aggregate_year_month_page(html_writer& w, const httplib::Request& req);
void overview_aggregate_year_fv_page(html_writer& w, const httplib::Request& req);
void overview_aggregate_month_page(html_writer& w, const httplib::Request& req);
void overview_year_page(html_writer& w, const httplib::Request& req);
void time_graph_savings_rate_page(html_writer& w);
void time_graph_tax_rate_page(html_writer& w);
void side_overview_page(html_writer& w, const httplib::Request& req);

} // end of namespace budget
