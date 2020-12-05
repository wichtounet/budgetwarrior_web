//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "writer.hpp"

namespace budget {

struct html_writer : writer {
    std::stringstream& os;

    html_writer(std::stringstream& os);

    virtual writer& operator<<(const std::string& value) override;
    virtual writer& operator<<(const double& value) override;

    virtual writer& operator<<(const budget::money& m) override;
    virtual writer& operator<<(const budget::month& m) override;
    virtual writer& operator<<(const budget::year& m) override;

    virtual writer& operator<<(const end_of_line_t& m) override;
    virtual writer& operator<<(const p_begin_t& m) override;
    virtual writer& operator<<(const p_end_t& m) override;
    virtual writer& operator<<(const title_begin_t& m) override;
    virtual writer& operator<<(const title_end_t& m) override;
    virtual writer& operator<<(const year_month_selector& m) override;
    virtual writer& operator<<(const year_selector& m) override;
    virtual writer& operator<<(const asset_selector& m) override;
    virtual writer& operator<<(const add_button& m) override;
    virtual writer& operator<<(const set_button& m) override;

    virtual bool is_web() override;

    virtual void display_table(std::vector<std::string>& columns, std::vector<std::vector<std::string>>& contents, size_t groups = 1, std::vector<size_t> lines = {}, size_t left = 0, size_t foot = 0) override;
    virtual void display_graph(const std::string& title, std::vector<std::string>& categories, std::vector<std::string> series_names, std::vector<std::vector<float>>& series_values) override;

    void defer_script(const std::string& script);
    void load_deferred_scripts();

    void use_module(const std::string& module);

private:
    std::vector<std::string> scripts;
    std::vector<std::string> modules;
    bool title_started = false;

    bool need_module(const std::string& module);
};

} //end of namespace budget
