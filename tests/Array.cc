#include <portable/Array.hh>
#include <snitch/snitch_all.hpp>

#include <cstring>

TEST_CASE("Array") {
    Array<char, 5> alloy;
    memcpy(alloy.values(), "alloy", alloy.count());
    Array<char, 5> loyal;
    memcpy(loyal.values(), "loyal", loyal.count());

    SECTION("count") {
        CHECK(alloy.count() == 5);
    }

    SECTION("values") {
        CHECK(alloy.values()[4] == 'y');
        alloy.values()[4] = 'w';
        CHECK(alloy.values()[4] == 'w');
    }

    SECTION("operator[]") {
        CHECK(alloy[4] == 'y');
        alloy[4] = 'w';
        CHECK(alloy[4] == 'w');
    }

    SECTION("rotateLeft") {
        alloy.rotateLeft(0);
        CHECK(alloy == alloy);
        alloy.rotateLeft(5);
        CHECK(alloy == alloy);
        alloy.rotateLeft(2);
        CHECK(alloy == loyal);
    }

    SECTION("rotateRight") {
        alloy.rotateRight(0);
        CHECK(alloy == alloy);
        alloy.rotateRight(5);
        CHECK(alloy == alloy);
        alloy.rotateRight(3);
        CHECK(alloy == loyal);
    }

    SECTION("operator==") {
        CHECK(alloy == alloy);
        CHECK_FALSE(alloy == loyal);
        CHECK_FALSE(loyal == alloy);
    }

    SECTION("operator!=") {
        CHECK_FALSE(alloy != alloy);
        CHECK(alloy != loyal);
        CHECK(loyal != alloy);
    }

    SECTION("operator<") {
        CHECK_FALSE(alloy < alloy);
        CHECK(alloy < loyal);
        CHECK_FALSE(loyal < alloy);
    }

    SECTION("operator>") {
        CHECK_FALSE(alloy > alloy);
        CHECK_FALSE(alloy > loyal);
        CHECK(loyal > alloy);
    }

    SECTION("operator<=") {
        CHECK(alloy <= alloy);
        CHECK(alloy <= loyal);
        CHECK_FALSE(loyal <= alloy);
    }

    SECTION("operator>=") {
        CHECK(alloy >= alloy);
        CHECK_FALSE(alloy >= loyal);
        CHECK(loyal >= alloy);
    }
}
