//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "api/server_api.hpp"
#include "api/retirement_api.hpp"

#include "assets.hpp"
#include "liabilities.hpp"
#include "accounts.hpp"
#include "data.hpp"
#include "data_cache.hpp"
#include "retirement.hpp"

#include "pages/web_config.hpp"

using namespace budget;

void budget::retirement_countdown_api(const httplib::Request& req, httplib::Response& res) {
    if (auto fi_expenses = budget::get_fi_expenses()){
        data_cache cache;

        const auto nw           = get_fi_net_worth(cache);
        const auto savings_rate = running_savings_rate(cache);
        const auto income       = running_income(cache);

        const auto currency     = get_default_currency();
        const auto wrate        = to_number<double>(internal_config_value("withdrawal_rate"));
        const auto roi          = to_number<double>(internal_config_value("expected_roi"));
        const auto years        = double(100.0 / wrate);
        const auto fi_goal         = years * fi_expenses;

        date_type fi_base_months = 0;
        {
            auto current_nw = nw;
            while (current_nw < fi_goal) {
                current_nw *= 1.0 + (roi / 100.0) / 12;
                current_nw += (savings_rate * income) / 12;

                ++fi_base_months;
            }
        }

        if (nw > fi_goal) {
            api_success_content(req, res, "You are FI!");
        } else {
            // TODO Add days granularity 

            const auto missing_years = unsigned(fi_base_months / 12);
            const auto missing_months = unsigned(fi_base_months - (missing_years * 12));

            auto result = std::format("{} years, {} months", missing_years, missing_months);

            api_success_content(req, res, result);
        }
    } else {
        api_success_content(req, res, "No expenses");
    }
}
