#include <portable/Array.hh>
#include <snitch/snitch_all.hpp>

#include <cstdio>

TEST_CASE("Array") {
    Array<char, 6> hello;
    snprintf(hello.values(), hello.count(), "hello");
    Array<char, 6> world;
    snprintf(world.values(), world.count(), "world");

    SECTION("operator==") {
        CHECK(hello == hello);
        CHECK_FALSE(hello == world);
        CHECK_FALSE(world == hello);
    }

    SECTION("operator!=") {
        CHECK_FALSE(hello != hello);
        CHECK(hello != world);
        CHECK(world != hello);
    }

    SECTION("operator<") {
        CHECK_FALSE(hello < hello);
        CHECK(hello < world);
        CHECK_FALSE(world < hello);
    }

    SECTION("operator>") {
        CHECK_FALSE(hello > hello);
        CHECK_FALSE(hello > world);
        CHECK(world > hello);
    }

    SECTION("operator<=") {
        CHECK(hello <= hello);
        CHECK(hello <= world);
        CHECK_FALSE(world <= hello);
    }

    SECTION("operator>=") {
        CHECK(hello >= hello);
        CHECK_FALSE(hello >= world);
        CHECK(world >= hello);
    }
}
