//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pages/server_pages.hpp"

namespace budget {

// Asset Shares Pages
void list_asset_shares_page(html_writer& w);
void add_asset_shares_page(html_writer& w);
void edit_asset_shares_page(html_writer& w, const httplib::Request& req);

} // end of namespace budget
