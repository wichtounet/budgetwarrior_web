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
}; // namespace httplib

namespace budget {

void add_expenses_api(const httplib::Request& req, httplib::Response& res);
void edit_expenses_api(const httplib::Request& req, httplib::Response& res);
void delete_expenses_api(const httplib::Request& req, httplib::Response& res);
void list_expenses_api(const httplib::Request& req, httplib::Response& res);
void import_expenses_api(const httplib::Request& req, httplib::Response& res);
void import_neon_expenses_api(const httplib::Request& req, httplib::Response& res);
void import_cembra_expenses_api(const httplib::Request& req, httplib::Response& res);

} // end of namespace budget
