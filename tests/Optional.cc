#include <portable/Optional.hh>
#include <snitch/snitch_all.hpp>

TEST_CASE("Optional") {
    Optional<u32> none;
    Optional<u32> zero(0);
    Optional<u32> one(1);

    SECTION("operator==") {
        CHECK(none == none);
        CHECK_FALSE(none == zero);
        CHECK_FALSE(none == 0);
        CHECK(zero == zero);
        CHECK(zero == 0);
        CHECK_FALSE(zero == one);
        CHECK_FALSE(zero == 1);
    }

    SECTION("operator!=") {
        CHECK_FALSE(none != none);
        CHECK(none != zero);
        CHECK(none != 0);
        CHECK_FALSE(zero != zero);
        CHECK_FALSE(zero != 0);
        CHECK(zero != one);
        CHECK(zero != 1);
    }
}
