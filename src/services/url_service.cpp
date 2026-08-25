#include "services/url_service.hpp"

#include "url_utils.hpp"

namespace url_shortener::services {

UrlService::UrlService(userver::storages::postgres::ClusterPtr pg_cluster)
    : repository_(std::move(pg_cluster)) {}

CreateResult UrlService::Create(std::string_view original_url,
                                std::string_view expires_at) const {
  if (!IsValidHttpUrl(original_url)) {
    return {CreateStatus::kInvalidUrl, std::nullopt};
  }
  if (!expires_at.empty() && !IsValidExpiration(expires_at)) {
    return {CreateStatus::kInvalidExpiration, std::nullopt};
  }

  constexpr int kMaxAttempts = 5;
  for (int attempt = 0; attempt < kMaxAttempts; ++attempt) {
    auto url =
        repository_.Insert(GenerateShortCode(), original_url, expires_at);
    if (url) {
      return {CreateStatus::kCreated, std::move(url)};
    }
  }
  return {CreateStatus::kUnavailable, std::nullopt};
}

std::optional<domain::Url> UrlService::Get(std::string_view short_code) const {
  return repository_.Find(short_code);
}

ResolveResult UrlService::Resolve(std::string_view short_code) const {
  const auto url = repository_.Find(short_code);
  if (!url) {
    return {ResolveStatus::kNotFound, {}};
  }
  if (url->expired) {
    return {ResolveStatus::kExpired, {}};
  }

  repository_.IncrementClicks(short_code);
  return {ResolveStatus::kFound, url->original_url};
}

bool UrlService::Delete(std::string_view short_code) const {
  return repository_.Delete(short_code);
}

}  // namespace url_shortener::services
