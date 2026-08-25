#include "observability/app_metrics.hpp"

#include <userver/components/statistics_storage.hpp>
#include <userver/utils/statistics/metric_tag.hpp>
#include <userver/utils/statistics/rate_counter.hpp>

namespace url_shortener::observability {

namespace {

using Counter = userver::utils::statistics::RateCounter;
using CounterTag = userver::utils::statistics::MetricTag<Counter>;

const CounterTag kUrlsCreated{"url_shortener.urls_created"};
const CounterTag kRedirectsCompleted{"url_shortener.redirects_completed"};
const CounterTag kUrlsDeleted{"url_shortener.urls_deleted"};
const CounterTag kNotFound{"url_shortener.not_found"};
const CounterTag kExpired{"url_shortener.expired"};
const CounterTag kValidationErrors{"url_shortener.validation_errors"};

}  // namespace

AppMetrics::AppMetrics(
    const userver::components::ComponentContext& component_context)
    : metrics_(component_context
                   .FindComponent<userver::components::StatisticsStorage>()
                   .GetMetricsStorage()) {}

void AppMetrics::UrlCreated() const { ++metrics_->GetMetric(kUrlsCreated); }

void AppMetrics::RedirectCompleted() const {
  ++metrics_->GetMetric(kRedirectsCompleted);
}

void AppMetrics::UrlDeleted() const { ++metrics_->GetMetric(kUrlsDeleted); }

void AppMetrics::NotFound() const { ++metrics_->GetMetric(kNotFound); }

void AppMetrics::Expired() const { ++metrics_->GetMetric(kExpired); }

void AppMetrics::ValidationError() const {
  ++metrics_->GetMetric(kValidationErrors);
}

}  // namespace url_shortener::observability
