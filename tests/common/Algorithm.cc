#include <common/Algorithm.hh>
#include <snitch/snitch_all.hpp>

#include <array>

TEST_CASE("Sort") {
    SECTION("0") {
        std::array<u32, 0> expected{};

        std::array<u32, 0> actual{};
        Sort(actual, actual.size());

        CHECK(actual == expected);
    }

    SECTION("1") {
        std::array<u32, 1> expected{117};

        std::array<u32, 1> actual{117};
        Sort(actual, actual.size());

        CHECK(actual == expected);
    }

    SECTION("2") {
        std::array<u32, 2> expected{29, 232};

        std::array<u32, 2> actual{232, 29};
        Sort(actual, actual.size());

        CHECK(actual == expected);
    }

    SECTION("3") {
        std::array<u32, 3> expected{101, 195, 236};

        std::array<u32, 3> actual{195, 101, 236};
        Sort(actual, actual.size());

        CHECK(actual == expected);
    }

    SECTION("4") {
        std::array<u32, 4> expected{10, 218, 222, 255};

        std::array<u32, 4> actual{255, 222, 218, 10};
        Sort(actual, actual.size());

        CHECK(actual == expected);
    }

    SECTION("5") {
        std::array<u32, 5> expected{29, 130, 183, 234, 249};

        std::array<u32, 5> actual{183, 249, 29, 130, 234};
        Sort(actual, actual.size());

        CHECK(actual == expected);
    }

    SECTION("6") {
        std::array<u32, 6> expected{23, 27, 140, 145, 176, 223};

        std::array<u32, 6> actual{223, 27, 140, 145, 176, 23};
        Sort(actual, actual.size());

        CHECK(actual == expected);
    }

    SECTION("7") {
        std::array<u32, 7> expected{9, 74, 126, 155, 191, 233, 247};

        std::array<u32, 7> actual{74, 155, 191, 9, 233, 247, 126};
        Sort(actual, actual.size());

        CHECK(actual == expected);
    }

    SECTION("8") {
        std::array<u32, 8> expected{0, 17, 17, 35, 123, 126, 144, 214};

        std::array<u32, 8> actual{17, 144, 123, 214, 126, 17, 35, 0};
        Sort(actual, actual.size());

        CHECK(actual == expected);
    }
}

TEST_CASE("Reverse") {
    SECTION("0") {
        std::array<u32, 0> expected{};

        std::array<u32, 0> actual{};
        Reverse(actual, actual.size());

        CHECK(actual == expected);
    }

    SECTION("1") {
        std::array<u32, 1> expected{117};

        std::array<u32, 1> actual{117};
        Reverse(actual, actual.size());

        CHECK(actual == expected);
    }

    SECTION("2") {
        std::array<u32, 2> expected{29, 232};

        std::array<u32, 2> actual{232, 29};
        Reverse(actual, actual.size());

        CHECK(actual == expected);
    }

    SECTION("3") {
        std::array<u32, 3> expected{236, 101, 195};

        std::array<u32, 3> actual{195, 101, 236};
        Reverse(actual, actual.size());

        CHECK(actual == expected);
    }

    SECTION("4") {
        std::array<u32, 4> expected{10, 218, 222, 255};

        std::array<u32, 4> actual{255, 222, 218, 10};
        Reverse(actual, actual.size());

        CHECK(actual == expected);
    }
}

TEST_CASE("RotateLeft") {
    std::array<u32, 5> actual{183, 249, 29, 130, 234};

    SECTION("0") {
        std::array<u32, 5> expected{183, 249, 29, 130, 234};

        RotateLeft(actual, actual.size(), 0);

        CHECK(actual == expected);
    }

    SECTION("1") {
        std::array<u32, 5> expected{249, 29, 130, 234, 183};

        RotateLeft(actual, actual.size(), 1);

        CHECK(actual == expected);
    }

    SECTION("2") {
        std::array<u32, 5> expected{29, 130, 234, 183, 249};

        RotateLeft(actual, actual.size(), 2);

        CHECK(actual == expected);
    }

    SECTION("3") {
        std::array<u32, 5> expected{130, 234, 183, 249, 29};

        RotateLeft(actual, actual.size(), 3);

        CHECK(actual == expected);
    }

    SECTION("4") {
        std::array<u32, 5> expected{234, 183, 249, 29, 130};

        RotateLeft(actual, actual.size(), 4);

        CHECK(actual == expected);
    }

    SECTION("5") {
        std::array<u32, 5> expected{183, 249, 29, 130, 234};

        RotateLeft(actual, actual.size(), 5);

        CHECK(actual == expected);
    }
}

TEST_CASE("RotateRight") {
    std::array<u32, 5> actual{183, 249, 29, 130, 234};

    SECTION("0") {
        std::array<u32, 5> expected{183, 249, 29, 130, 234};

        RotateRight(actual, actual.size(), 0);

        CHECK(actual == expected);
    }

    SECTION("1") {
        std::array<u32, 5> expected{234, 183, 249, 29, 130};

        RotateRight(actual, actual.size(), 1);

        CHECK(actual == expected);
    }

    SECTION("2") {
        std::array<u32, 5> expected{130, 234, 183, 249, 29};

        RotateRight(actual, actual.size(), 2);

        CHECK(actual == expected);
    }

    SECTION("3") {
        std::array<u32, 5> expected{29, 130, 234, 183, 249};

        RotateRight(actual, actual.size(), 3);

        CHECK(actual == expected);
    }

    SECTION("4") {
        std::array<u32, 5> expected{249, 29, 130, 234, 183};

        RotateRight(actual, actual.size(), 4);

        CHECK(actual == expected);
    }

    SECTION("5") {
        std::array<u32, 5> expected{183, 249, 29, 130, 234};

        RotateRight(actual, actual.size(), 5);

        CHECK(actual == expected);
    }
}
