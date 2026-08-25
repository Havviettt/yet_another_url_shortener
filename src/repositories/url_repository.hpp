#pragma once

#include "domain/url.hpp"

#include <optional>
#include <string_view>

#include <userver/storages/postgres/cluster.hpp>

namespace url_shortener::repositories {

class UrlRepository final {
 public:
  explicit UrlRepository(userver::storages::postgres::ClusterPtr pg_cluster);

  std::optional<domain::Url> Insert(std::string_view short_code,
                                    std::string_view original_url,
                                    std::string_view expires_at) const;
  std::optional<domain::Url> Find(std::string_view short_code) const;
  void IncrementClicks(std::string_view short_code) const;
  bool Delete(std::string_view short_code) const;

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace url_shortener::repositories
