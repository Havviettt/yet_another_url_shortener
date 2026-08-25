#include "repositories/url_repository.hpp"

#include <string>
#include <utility>

#include <userver/storages/postgres/result_set.hpp>

namespace url_shortener::repositories {

namespace {

domain::Url ParseUrl(const userver::storages::postgres::Row& row) {
  domain::Url url;
  url.short_code = row["short_code"].As<std::string>();
  url.original_url = row["original_url"].As<std::string>();
  url.created_at = row["created_at"].As<std::string>();
  url.expires_at = row["expires_at"].As<std::optional<std::string>>();
  url.clicks_count = row["clicks_count"].As<std::int64_t>();
  url.expired = row["expired"].As<bool>();
  return url;
}

}  // namespace

UrlRepository::UrlRepository(userver::storages::postgres::ClusterPtr pg_cluster)
    : pg_cluster_(std::move(pg_cluster)) {}

std::optional<domain::Url> UrlRepository::Insert(
    std::string_view short_code, std::string_view original_url,
    std::string_view expires_at) const {
  const auto result = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      "INSERT INTO url_shortener.urls(short_code, original_url, expires_at) "
      "VALUES($1, $2, NULLIF($3, '')::timestamptz) "
      "ON CONFLICT (short_code) DO NOTHING "
      "RETURNING short_code, original_url, created_at::text, "
      "expires_at::text, clicks_count, false AS expired",
      short_code, original_url, expires_at);

  if (result.IsEmpty()) {
    return std::nullopt;
  }
  return ParseUrl(result[0]);
}

std::optional<domain::Url> UrlRepository::Find(
    std::string_view short_code) const {
  const auto result = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kSlave,
      "SELECT short_code, original_url, created_at::text, expires_at::text, "
      "clicks_count, expires_at IS NOT NULL AND expires_at <= NOW() AS expired "
      "FROM url_shortener.urls WHERE short_code = $1",
      short_code);

  if (result.IsEmpty()) {
    return std::nullopt;
  }
  return ParseUrl(result[0]);
}

void UrlRepository::IncrementClicks(std::string_view short_code) const {
  pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      "UPDATE url_shortener.urls SET clicks_count = clicks_count + 1 "
      "WHERE short_code = $1",
      short_code);
}

bool UrlRepository::Delete(std::string_view short_code) const {
  const auto result = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      "DELETE FROM url_shortener.urls WHERE short_code = $1", short_code);
  return result.RowsAffected() > 0;
}

}  // namespace url_shortener::repositories
