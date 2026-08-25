#pragma once

#include <userver/components/component_context.hpp>
#include <userver/utils/statistics/fwd.hpp>

namespace url_shortener::observability {

class AppMetrics final {
 public:
  explicit AppMetrics(
      const userver::components::ComponentContext& component_context);

  void UrlCreated() const;
  void RedirectCompleted() const;
  void UrlDeleted() const;
  void NotFound() const;
  void Expired() const;
  void ValidationError() const;

 private:
  userver::utils::statistics::MetricsStoragePtr metrics_;
};

}  // namespace url_shortener::observability
