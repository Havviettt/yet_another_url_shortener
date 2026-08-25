#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace url_shortener::domain {

struct Url {
  std::string short_code;
  std::string original_url;
  std::string created_at;
  std::optional<std::string> expires_at;
  std::int64_t clicks_count{0};
  bool expired{false};
};

}  // namespace url_shortener::domain
