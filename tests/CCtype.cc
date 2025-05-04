#include <portable/CCtype.hh>
#include <snitch/snitch_all.hpp>

TEST_CASE("CCtype") {
    SECTION("Isdigit") {
        CHECK(CCtype::Isdigit('0'));
        CHECK_FALSE(CCtype::Isdigit('A'));
        CHECK_FALSE(CCtype::Isdigit('a'));
        CHECK_FALSE(CCtype::Isdigit('~'));
    }

    SECTION("Tolower") {
        CHECK(CCtype::Tolower('0') == '0');
        CHECK(CCtype::Tolower('A') == 'a');
        CHECK(CCtype::Tolower('a') == 'a');
        CHECK(CCtype::Tolower('~') == '~');
    }
}
