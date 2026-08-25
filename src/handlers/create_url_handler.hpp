#pragma once

#include "observability/app_metrics.hpp"
#include "services/url_service.hpp"

#include <userver/server/handlers/http_handler_base.hpp>

namespace url_shortener::handlers {

class CreateUrlHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-create-url";

  CreateUrlHandler(const userver::components::ComponentConfig&,
                   const userver::components::ComponentContext&);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  services::UrlService service_;
  observability::AppMetrics metrics_;
};

}  // namespace url_shortener::handlers
