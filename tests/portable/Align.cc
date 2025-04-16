#include <portable/Align.hh>
#include <snitch/snitch_all.hpp>

TEST_CASE("IsAligned") {
    CHECK_FALSE(IsAligned(5, 2));
    CHECK(IsAligned(6, 2));
}

TEST_CASE("AlignDown") {
    CHECK(AlignDown(5, 2) == 4);
    CHECK(AlignDown(6, 2) == 6);
}

TEST_CASE("AlignUp") {
    CHECK(AlignUp(5, 2) == 6);
    CHECK(AlignUp(6, 2) == 6);
}
