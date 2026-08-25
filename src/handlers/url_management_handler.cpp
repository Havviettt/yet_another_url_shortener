#include "handlers/url_management_handler.hpp"

#include "http/http_utils.hpp"

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/http/content_type.hpp>
#include <userver/logging/log.hpp>
#include <userver/logging/log_extra.hpp>
#include <userver/server/http/http_method.hpp>
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

UrlManagementHandler::UrlManagementHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context)
    : HttpHandlerBase(config, component_context),
      service_(GetCluster(component_context)),
      metrics_(component_context) {}

std::string UrlManagementHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  const auto request_id = http::EnsureRequestId(request);
  const auto& short_code = request.GetPathArg("short_code");
  if (request.GetMethod() == userver::server::http::HttpMethod::kDelete) {
    return Delete(request, request_id, short_code);
  }
  return Get(request, request_id, short_code);
}

std::string UrlManagementHandler::Get(
    const userver::server::http::HttpRequest& request,
    std::string_view request_id, std::string_view short_code) const {
  const auto url = service_.Get(short_code);
  if (!url) {
    metrics_.NotFound();
    return http::MakeError(request,
                           userver::server::http::HttpStatus::kNotFound,
                           "url_not_found", "short URL not found", request_id);
  }

  userver::formats::json::ValueBuilder body;
  body["short_code"] = url->short_code;
  body["short_url"] = "/" + url->short_code;
  body["original_url"] = url->original_url;
  body["created_at"] = url->created_at;
  body["clicks_count"] = url->clicks_count;
  body["expired"] = url->expired;
  if (url->expires_at) {
    body["expires_at"] = *url->expires_at;
  } else {
    body["expires_at"] = nullptr;
  }

  request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);
  return userver::formats::json::ToString(body.ExtractValue());
}

std::string UrlManagementHandler::Delete(
    const userver::server::http::HttpRequest& request,
    std::string_view request_id, std::string_view short_code) const {
  if (!service_.Delete(short_code)) {
    metrics_.NotFound();
    return http::MakeError(request,
                           userver::server::http::HttpStatus::kNotFound,
                           "url_not_found", "short URL not found", request_id);
  }

  request.GetHttpResponse().SetStatus(
      userver::server::http::HttpStatus::kNoContent);
  metrics_.UrlDeleted();
  LOG_INFO() << "Short URL deleted"
             << userver::logging::LogExtra{{"request_id", request_id},
                                           {"short_code", short_code}};
  return {};
}

}  // namespace url_shortener::handlers
