#include <portable/CStrings.hh>
#include <snitch/snitch_all.hpp>

TEST_CASE("CStrings") {
    SECTION("Strncasecmp") {
        char s[] = "Xyxz";
        CHECK(CStrings::Strncasecmp(s + 0, s + 0, 0) == 0);
        CHECK(CStrings::Strncasecmp(s + 0, s + 0, 1) == 0);
        CHECK(CStrings::Strncasecmp(s + 0, s + 0, 4) == 0);
        CHECK(CStrings::Strncasecmp(s + 0, s + 0, 5) == 0);
        CHECK(CStrings::Strncasecmp(s + 0, s + 0, 6) == 0);
        CHECK(CStrings::Strncasecmp(s + 0, s + 2, 0) == 0);
        CHECK(CStrings::Strncasecmp(s + 2, s + 0, 0) == 0);
        CHECK(CStrings::Strncasecmp(s + 0, s + 2, 1) == 0);
        CHECK(CStrings::Strncasecmp(s + 2, s + 0, 1) == 0);
        CHECK(CStrings::Strncasecmp(s + 0, s + 2, 2) < 0);
        CHECK(CStrings::Strncasecmp(s + 0, s + 2, 3) < 0);
        CHECK(CStrings::Strncasecmp(s + 2, s + 0, 2) > 0);
        CHECK(CStrings::Strncasecmp(s + 2, s + 0, 3) > 0);
    }
}
