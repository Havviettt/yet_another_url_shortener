#include "handlers/redirect_handler.hpp"

#include "http/http_utils.hpp"

#include <string_view>

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

RedirectHandler::RedirectHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context)
    : HttpHandlerBase(config, component_context),
      service_(GetCluster(component_context)),
      metrics_(component_context) {}

std::string RedirectHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  const auto request_id = http::EnsureRequestId(request);
  const auto& short_code = request.GetPathArg("short_code");
  const auto result = service_.Resolve(short_code);

  if (result.status == services::ResolveStatus::kNotFound) {
    metrics_.NotFound();
    return http::MakeError(request,
                           userver::server::http::HttpStatus::kNotFound,
                           "url_not_found", "short URL not found", request_id);
  }
  if (result.status == services::ResolveStatus::kExpired) {
    metrics_.Expired();
    return http::MakeError(request, userver::server::http::HttpStatus::kGone,
                           "url_expired", "short URL has expired", request_id);
  }

  auto& response = request.GetHttpResponse();
  response.SetStatus(userver::server::http::HttpStatus::kFound);
  response.SetHeader(std::string_view{"Location"}, result.original_url);
  metrics_.RedirectCompleted();
  LOG_INFO() << "Short URL resolved"
             << userver::logging::LogExtra{{"request_id", request_id},
                                           {"short_code", short_code}};
  return {};
}

}  // namespace url_shortener::handlers
