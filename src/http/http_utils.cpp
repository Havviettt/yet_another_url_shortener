#include "http/http_utils.hpp"

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/http/content_type.hpp>
#include <userver/utils/uuid4.hpp>

namespace url_shortener::http {

std::string EnsureRequestId(const userver::server::http::HttpRequest& request) {
  auto request_id = request.GetHeader(std::string_view{"X-Request-ID"});
  if (request_id.empty()) {
    request_id = userver::utils::generators::GenerateUuid();
  }
  request.GetHttpResponse().SetHeader(std::string_view{"X-Request-ID"},
                                      request_id);
  return request_id;
}

std::string MakeError(const userver::server::http::HttpRequest& request,
                      userver::server::http::HttpStatus status,
                      std::string_view code, std::string_view message,
                      std::string_view request_id) {
  auto& response = request.GetHttpResponse();
  response.SetStatus(status);
  response.SetContentType(userver::http::content_type::kApplicationJson);

  userver::formats::json::ValueBuilder body;
  body["error"]["code"] = code;
  body["error"]["message"] = message;
  body["request_id"] = request_id;
  return userver::formats::json::ToString(body.ExtractValue());
}

}  // namespace url_shortener::http
