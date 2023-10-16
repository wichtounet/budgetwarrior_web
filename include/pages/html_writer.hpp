//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <regex>

#include "date.hpp"
#include "writer.hpp"

namespace budget {

struct html_writer : writer {
    std::stringstream& os;

    explicit html_writer(std::stringstream& os) : os(os){}

    virtual ~html_writer() = default;

    writer& operator<<(std::string_view value) override;
    writer& operator<<(double value) override;
    writer& operator<<(size_t value) override;
    writer& operator<<(long value) override;
    writer& operator<<(int value) override;
    writer& operator<<(unsigned value) override;

    writer& operator<<(const budget::money& m) override;
    writer& operator<<(const budget::year& y) override;
    writer& operator<<(const budget::month& m) override;
    writer& operator<<(const budget::day& d) override;

    writer& operator<<(const end_of_line_t& m) override;
    writer& operator<<(const p_begin_t& m) override;
    writer& operator<<(const p_end_t& m) override;
    writer& operator<<(const title_begin_t& m) override;
    writer& operator<<(const title_end_t& m) override;
    writer& operator<<(const year_month_selector& m) override;
    writer& operator<<(const year_selector& m) override;
    writer& operator<<(const asset_selector& m) override;
    writer& operator<<(const active_asset_selector& m) override;
    writer& operator<<(const add_button& m) override;
    writer& operator<<(const set_button& m) override;

    bool is_web() override;

    void display_table(std::vector<std::string>&              columns,
                       std::vector<std::vector<std::string>>& contents,
                       size_t                                 groups = 1,
                       std::vector<size_t>                    lines  = {},
                       size_t                                 left   = 0,
                       size_t                                 foot   = 0) override;
    void display_graph(std::string_view               title,
                       std::vector<std::string>&        categories,
                       std::vector<std::string>         series_names,
                       std::vector<std::vector<float>>& series_values) override;

    void defer_script(const std::string& script);
    void load_deferred_scripts();

    void use_module(const std::string& module);

private:
    std::vector<std::string> scripts;
    std::vector<std::string> modules;
    bool                     title_started = false;

    bool need_module(const std::string& module);
};

// Very small utility functions necessary to convert regex matches (from httplib) to a number

template <typename T, typename It>
T to_number(const std::sub_match<It>& sm) {
    return to_number<T>(sm.str());
}

template <typename It>
year year_from_string(const std::sub_match<It>& sm) {
    return year_from_string(sm.str());
}

template <typename It>
month month_from_string(const std::sub_match<It>& sm) {
    return month_from_string(sm.str());
}

template <typename It>
day day_from_string(const std::sub_match<It>& sm) {
    return day_from_string(sm.str());
}

} // end of namespace budget
