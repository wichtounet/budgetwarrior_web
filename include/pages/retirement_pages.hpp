//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "pages/server_pages.hpp"

namespace budget {

void retirement_status_page(html_writer& w);
void retirement_configure_page(html_writer& w);
void retirement_fi_ratio_over_time(html_writer& w);

} // end of namespace budget
