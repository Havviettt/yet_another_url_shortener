#pragma once

#include <string>
#include <string_view>

#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_status.hpp>

namespace url_shortener::http {

std::string EnsureRequestId(const userver::server::http::HttpRequest& request);

std::string MakeError(const userver::server::http::HttpRequest& request,
                      userver::server::http::HttpStatus status,
                      std::string_view code, std::string_view message,
                      std::string_view request_id);

}  // namespace url_shortener::http
