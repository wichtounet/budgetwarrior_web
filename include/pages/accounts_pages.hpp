//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pages/server_pages.hpp"

namespace budget {

void accounts_page(html_writer& w);
void all_accounts_page(html_writer& w);
void add_accounts_page(html_writer& w);
void edit_accounts_page(html_writer& w, const httplib::Request& req);
void archive_accounts_month_page(html_writer& w);
void archive_accounts_year_page(html_writer& w);

} // end of namespace budget
