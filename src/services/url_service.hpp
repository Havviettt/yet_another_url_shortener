#pragma once

#include "domain/url.hpp"
#include "repositories/url_repository.hpp"

#include <optional>
#include <string>
#include <string_view>

#include <userver/storages/postgres/cluster.hpp>

namespace url_shortener::services {

enum class CreateStatus {
  kCreated,
  kInvalidUrl,
  kInvalidExpiration,
  kUnavailable
};

struct CreateResult {
  CreateStatus status;
  std::optional<domain::Url> url;
};

enum class ResolveStatus { kFound, kNotFound, kExpired };

struct ResolveResult {
  ResolveStatus status;
  std::string original_url;
};

class UrlService final {
 public:
  explicit UrlService(userver::storages::postgres::ClusterPtr pg_cluster);

  CreateResult Create(std::string_view original_url,
                      std::string_view expires_at) const;
  std::optional<domain::Url> Get(std::string_view short_code) const;
  ResolveResult Resolve(std::string_view short_code) const;
  bool Delete(std::string_view short_code) const;

 private:
  repositories::UrlRepository repository_;
};

}  // namespace url_shortener::services
