#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace url_shortener {

inline constexpr std::size_t kShortCodeLength = 7;

bool IsValidHttpUrl(std::string_view url);
bool IsValidExpiration(std::string_view expires_at);
std::string GenerateShortCode(std::size_t length = kShortCodeLength);

}  // namespace url_shortener
