#include "handlers/create_url_handler.hpp"

#include "http/http_utils.hpp"

#include <exception>
#include <string>

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/http/content_type.hpp>
#include <userver/logging/log.hpp>
#include <userver/logging/log_extra.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/component.hpp>

namespace url_shortener::handlers {

namespace {

userver::storages::postgres::ClusterPtr GetCluster(
    const userver::components::ComponentContext& context) {
  return context.FindComponent<userver::components::Postgres>("postgres-db-1")
      .GetCluster();
}

}  // namespace

CreateUrlHandler::CreateUrlHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context)
    : HttpHandlerBase(config, component_context),
      service_(GetCluster(component_context)),
      metrics_(component_context) {}

std::string CreateUrlHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  const auto request_id = http::EnsureRequestId(request);
  std::string original_url;
  std::string expires_at;
  try {
    const auto json = userver::formats::json::FromString(request.RequestBody());
    original_url = json["url"].As<std::string>();
    if (json.HasMember("expires_at") && !json["expires_at"].IsNull()) {
      expires_at = json["expires_at"].As<std::string>();
    }
  } catch (const std::exception&) {
    metrics_.ValidationError();
    return http::MakeError(
        request, userver::server::http::HttpStatus::kBadRequest,
        "invalid_request", "request body must contain a string field 'url'",
        request_id);
  }

  const auto result = service_.Create(original_url, expires_at);
  if (result.status == services::CreateStatus::kInvalidUrl) {
    metrics_.ValidationError();
    return http::MakeError(
        request, userver::server::http::HttpStatus::kBadRequest, "invalid_url",
        "url must be a valid http or https URL", request_id);
  }
  if (result.status == services::CreateStatus::kInvalidExpiration) {
    metrics_.ValidationError();
    return http::MakeError(
        request, userver::server::http::HttpStatus::kBadRequest,
        "invalid_expiration", "expires_at must be an RFC 3339 UTC timestamp",
        request_id);
  }
  if (result.status == services::CreateStatus::kUnavailable) {
    return http::MakeError(
        request, userver::server::http::HttpStatus::kServiceUnavailable,
        "code_generation_failed", "could not generate a unique short code",
        request_id);
  }

  const auto& url = *result.url;
  userver::formats::json::ValueBuilder body;
  body["short_code"] = url.short_code;
  body["short_url"] = "/" + url.short_code;
  body["original_url"] = url.original_url;
  if (url.expires_at) {
    body["expires_at"] = *url.expires_at;
  }

  auto& response = request.GetHttpResponse();
  response.SetStatus(userver::server::http::HttpStatus::kCreated);
  response.SetContentType(userver::http::content_type::kApplicationJson);
  metrics_.UrlCreated();
  LOG_INFO() << "Short URL created"
             << userver::logging::LogExtra{{"request_id", request_id},
                                           {"short_code", url.short_code}};
  return userver::formats::json::ToString(body.ExtractValue());
}

}  // namespace url_shortener::handlers
