//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "config.hpp"
#include "money.hpp"

namespace budget {

bool          is_side_hustle_enabled();
budget::money get_fi_expenses();

} // end of namespace budget
