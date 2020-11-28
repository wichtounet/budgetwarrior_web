//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

namespace httplib {
struct Request;
struct Response;
};

namespace budget {

void add_accounts_api(const httplib::Request& req, httplib::Response& res);
void edit_accounts_api(const httplib::Request& req, httplib::Response& res);
void delete_accounts_api(const httplib::Request& req, httplib::Response& res);
void list_accounts_api(const httplib::Request& req, httplib::Response& res);
void archive_accounts_month_api(const httplib::Request& req, httplib::Response& res);
void archive_accounts_year_api(const httplib::Request& req, httplib::Response& res);

} //end of namespace budget
