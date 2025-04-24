#include <portable/String.hh>
#include <snitch/snitch_all.hpp>

TEST_CASE("String") {
    SECTION("StartsWith") {
        CHECK(String::StartsWith("abc", ""));
        CHECK(String::StartsWith("abc", "a"));
        CHECK(String::StartsWith("abc", "ab"));
        CHECK(String::StartsWith("abc", "abc"));
        CHECK_FALSE(String::StartsWith("abc", "b"));
        CHECK_FALSE(String::StartsWith("abc", "c"));
        CHECK_FALSE(String::StartsWith("abc", "ac"));
        CHECK_FALSE(String::StartsWith("abc", "bc"));
        CHECK_FALSE(String::StartsWith("abc", "abcd"));
    }

    SECTION("EndsWith") {
        CHECK(String::EndsWith("abc", ""));
        CHECK(String::EndsWith("abc", "c"));
        CHECK(String::EndsWith("abc", "bc"));
        CHECK(String::EndsWith("abc", "abc"));
        CHECK_FALSE(String::EndsWith("abc", "b"));
        CHECK_FALSE(String::EndsWith("abc", "a"));
        CHECK_FALSE(String::EndsWith("abc", "ac"));
        CHECK_FALSE(String::EndsWith("abc", "ab"));
        CHECK_FALSE(String::EndsWith("abc", "abcd"));
    }
}
