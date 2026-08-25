#include "url_utils.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <random>
#include <regex>

namespace url_shortener {

namespace {

constexpr std::string_view kAlphabet =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

}  // namespace

bool IsValidHttpUrl(std::string_view url) {
  constexpr std::size_t kMaxUrlLength = 2048;
  if (url.empty() || url.size() > kMaxUrlLength) {
    return false;
  }

  std::string_view remainder;
  if (url.starts_with("https://")) {
    remainder = url.substr(8);
  } else if (url.starts_with("http://")) {
    remainder = url.substr(7);
  } else {
    return false;
  }

  const auto host_end = remainder.find_first_of("/?:#");
  const auto host = remainder.substr(0, host_end);
  if (host.empty()) {
    return false;
  }

  return std::none_of(url.begin(), url.end(), [](unsigned char ch) {
    return std::isspace(ch) || std::iscntrl(ch);
  });
}

bool IsValidExpiration(std::string_view expires_at) {
  static const std::regex kRfc3339Utc{
      R"(^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})Z$)"};
  std::cmatch match;
  if (!std::regex_match(expires_at.begin(), expires_at.end(), match,
                        kRfc3339Utc)) {
    return false;
  }

  const auto year = std::stoi(match[1].str());
  const auto month = std::stoi(match[2].str());
  const auto day = std::stoi(match[3].str());
  const auto hour = std::stoi(match[4].str());
  const auto minute = std::stoi(match[5].str());
  const auto second = std::stoi(match[6].str());
  const auto date = std::chrono::year_month_day{
      std::chrono::year{year}, std::chrono::month{static_cast<unsigned>(month)},
      std::chrono::day{static_cast<unsigned>(day)}};

  return year >= 1 && date.ok() && hour <= 23 && minute <= 59 && second <= 59;
}

std::string GenerateShortCode(std::size_t length) {
  thread_local std::mt19937_64 generator{std::random_device{}()};
  std::uniform_int_distribution<std::size_t> distribution(0,
                                                          kAlphabet.size() - 1);

  std::string code;
  code.reserve(length);
  for (std::size_t i = 0; i < length; ++i) {
    code.push_back(kAlphabet[distribution(generator)]);
  }
  return code;
}

}  // namespace url_shortener
