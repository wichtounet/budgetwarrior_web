//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <string>
#include <vector>

#include "date.hpp"
#include "money.hpp"

namespace httplib {
struct Server;
struct Response;
struct Request;
};

namespace budget {

struct writer;
struct html_writer;

void load_pages(httplib::Server& server);

bool authenticate(const httplib::Request& req, httplib::Response& res);
bool page_start(const httplib::Request& req, httplib::Response& res, std::stringstream& content_stream, const std::string& title);
void page_end(budget::html_writer& w, const httplib::Request& req, httplib::Response& res);
bool validate_parameters(html_writer& w, const httplib::Request& req, std::vector<const char*> parameters);

void display_error_message(budget::writer& w, const std::string& message);

void make_tables_sortable(budget::html_writer& w);

// Forms
void form_begin(budget::writer& w, std::string_view action, std::string_view back_page);
void page_form_begin(budget::writer& w, std::string_view action);
void form_begin_edit(budget::writer& w, std::string_view action, std::string_view back_page, std::string_view input_id);
void form_end(budget::writer& w, std::string_view button = "");

void add_text_picker(budget::writer& w, std::string_view title, std::string_view name, std::string_view default_value, bool required = true);
void add_password_picker(budget::writer& w, const std::string& title, const std::string& name, const std::string& default_value, bool required = true);
void add_name_picker(budget::writer& w, const std::string& default_value = "");
void add_title_picker(budget::writer& w, const std::string& default_value = "");
void add_amount_picker(budget::writer& w, const std::string& default_value = "");
void add_paid_amount_picker(budget::writer& w, const std::string& default_value = "");
void add_yes_no_picker(budget::writer& w, const std::string& title, const std::string& name, bool default_value);
void add_paid_picker(budget::writer& w, bool paid);
void add_date_picker(budget::writer& w, const std::string& default_value = "", bool one_line = false);
void add_account_picker(budget::writer& w, budget::date day, const std::string& default_value = "");
void add_account_picker_by_name(
        budget::writer& w, budget::date day, const std::string& title, const std::string& default_value, const std::string& input, bool allow_empty = false);
void add_share_asset_picker(budget::writer& w, const std::string& default_value = "");
void add_value_asset_picker(budget::writer& w, const std::string& default_value = "");
void add_active_share_asset_picker(budget::writer& w, const std::string& default_value = "");
void add_active_value_asset_picker(budget::writer& w, const std::string& default_value = "");
void add_liability_picker(budget::writer& w, const std::string& default_value = "");
void add_money_picker(budget::writer& w, const std::string& title, const std::string& name,
                      const std::string& default_value, bool required = true, bool one_line = false, const std::string& currency = "");
void add_integer_picker(budget::writer& w, const std::string& title, const std::string& name, bool negative,
                        const std::string& default_value = "");

// Charts
std::stringstream start_chart_base(budget::html_writer& w, const std::string& chart_type,
                                   const std::string& id = "container", std::string style = "");
std::stringstream start_chart(budget::html_writer& w, const std::string& title, const std::string& chart_type,
                              const std::string& id = "container", std::string style = "");
std::stringstream start_time_chart(budget::html_writer& w, const std::string& title, const std::string& chart_type,
                                   const std::string& id = "container", std::string style = "");
void end_chart(budget::html_writer& w, std::stringstream& ss);
void add_average_12_serie(std::stringstream& ss,
                         std::vector<budget::money> serie,
                         std::vector<std::string> dates);
void add_average_5_serie(std::stringstream& ss,
                         std::vector<budget::money> serie,
                         std::vector<std::string> dates);

} //end of namespace budget
