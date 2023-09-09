//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "incomes.hpp"

#include "pages/html_writer.hpp"
#include "pages/incomes_pages.hpp"
#include "http.hpp"

using namespace budget;

void budget::incomes_page(html_writer& w) {
    budget::show_incomes(w);

    make_tables_sortable(w);
}

void budget::set_incomes_page(html_writer& w) {
    w << title_begin << "Set income" << title_end;

    form_begin(w, "/api/incomes/add/", "/incomes/set/");

    add_amount_picker(w);

    form_end(w);
}
