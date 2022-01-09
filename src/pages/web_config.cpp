//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "pages/web_config.hpp"

using namespace budget;

bool budget::is_side_hustle_enabled() {
    return !user_config_value("side_category", "").empty();
}

budget::money budget::get_fi_expenses() {
    budget::money expenses;

    auto config = user_config_value("fi_expenses", "");
    if (!config.empty()) {
        expenses = budget::money_from_string(config);
    }

    return expenses;
}
