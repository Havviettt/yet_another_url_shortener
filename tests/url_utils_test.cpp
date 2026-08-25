#include "url_utils.hpp"

#include <algorithm>
#include <cctype>

#include <userver/utest/utest.hpp>

UTEST(UrlValidation, AcceptsHttpAndHttps) {
  EXPECT_TRUE(url_shortener::IsValidHttpUrl("https://example.com/long/path"));
  EXPECT_TRUE(url_shortener::IsValidHttpUrl("http://localhost:8080/test"));
}

UTEST(UrlValidation, RejectsInvalidUrls) {
  EXPECT_FALSE(url_shortener::IsValidHttpUrl("example.com"));
  EXPECT_FALSE(url_shortener::IsValidHttpUrl("ftp://example.com"));
  EXPECT_FALSE(url_shortener::IsValidHttpUrl("https:///missing-host"));
  EXPECT_FALSE(url_shortener::IsValidHttpUrl("https://example.com/with space"));
}

UTEST(ShortCode, HasExpectedShape) {
  const auto code = url_shortener::GenerateShortCode();
  EXPECT_EQ(code.size(), url_shortener::kShortCodeLength);
  EXPECT_TRUE(std::all_of(code.begin(), code.end(),
                          [](unsigned char ch) { return std::isalnum(ch); }));
}

UTEST(ExpirationValidation, AcceptsUtcRfc3339) {
  EXPECT_TRUE(url_shortener::IsValidExpiration("2030-12-31T23:59:59Z"));
}

UTEST(ExpirationValidation, RejectsInvalidValues) {
  EXPECT_FALSE(url_shortener::IsValidExpiration("2030-12-31"));
  EXPECT_FALSE(url_shortener::IsValidExpiration("2030-13-31T23:59:59Z"));
  EXPECT_FALSE(url_shortener::IsValidExpiration("2026-02-31T12:00:00Z"));
  EXPECT_FALSE(url_shortener::IsValidExpiration("0000-01-01T00:00:00Z"));
  EXPECT_FALSE(url_shortener::IsValidExpiration("tomorrow"));
}
