//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "pages/report_pages.hpp"
#include "pages/html_writer.hpp"
#include "http.hpp"
#include "report.hpp"

using namespace budget;

void budget::report_page(html_writer& w) {
    auto today = budget::local_day();
    report(w, today.year(), false, "");
}
