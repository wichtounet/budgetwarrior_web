//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pages/server_pages.hpp"

namespace budget {

void time_graph_income_page(html_writer & w);
void time_graph_earnings_page(html_writer & w);
void add_earnings_page(html_writer & w);
void edit_earnings_page(html_writer & w, const httplib::Request& req);
void earnings_page(html_writer & w, const httplib::Request& req);
void search_earnings_page(html_writer & w, const httplib::Request& req);
void all_earnings_page(html_writer & w);

void month_breakdown_income_graph(budget::html_writer& w, std::string_view title,
                                  budget::month month, budget::year year, bool mono = false,
                                  std::string_view style = "");

} //end of namespace budget
