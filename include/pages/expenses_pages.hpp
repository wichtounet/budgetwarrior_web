//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pages/server_pages.hpp"

namespace budget {

void expenses_page(html_writer& w, const httplib::Request& req);
void search_expenses_page(html_writer& w, const httplib::Request& req);
void time_graph_expenses_page(html_writer& w);
void all_expenses_page(html_writer& w);
void month_breakdown_expenses_page(html_writer& w, const httplib::Request& req);
void year_breakdown_expenses_page(html_writer& w, const httplib::Request& req);
void add_expenses_page(html_writer& w);
void edit_expenses_page(html_writer& w, const httplib::Request& req);
void import_expenses_page(html_writer& w);

void month_breakdown_expenses_graph(
        budget::html_writer& w, std::string_view title, budget::month month, budget::year year, bool mono = false, std::string_view style = "");

} // end of namespace budget
