#include <common/Algorithm.hh>
#include <vendor/lest.hpp>

static lest::tests specification;

#define CASE(name) lest_CASE(specification, name)

CASE("Sort") {
    SETUP("Placeholder") {
        SECTION("0") {
            std::array<u32, 0> expected{};

            std::array<u32, 0> actual{};
            Sort(actual, actual.size());

            EXPECT(actual == expected);
        }

        SECTION("1") {
            std::array<u32, 1> expected{117};

            std::array<u32, 1> actual{117};
            Sort(actual, actual.size());

            EXPECT(actual == expected);
        }

        SECTION("2") {
            std::array<u32, 2> expected{29, 232};

            std::array<u32, 2> actual{232, 29};
            Sort(actual, actual.size());

            EXPECT(actual == expected);
        }

        SECTION("3") {
            std::array<u32, 3> expected{101, 195, 236};

            std::array<u32, 3> actual{195, 101, 236};
            Sort(actual, actual.size());

            EXPECT(actual == expected);
        }

        SECTION("4") {
            std::array<u32, 4> expected{10, 218, 222, 255};

            std::array<u32, 4> actual{255, 222, 218, 10};
            Sort(actual, actual.size());

            EXPECT(actual == expected);
        }

        SECTION("5") {
            std::array<u32, 5> expected{29, 130, 183, 234, 249};

            std::array<u32, 5> actual{183, 249, 29, 130, 234};
            Sort(actual, actual.size());

            EXPECT(actual == expected);
        }

        SECTION("6") {
            std::array<u32, 6> expected{23, 27, 140, 145, 176, 223};

            std::array<u32, 6> actual{223, 27, 140, 145, 176, 23};
            Sort(actual, actual.size());

            EXPECT(actual == expected);
        }

        SECTION("7") {
            std::array<u32, 7> expected{9, 74, 126, 155, 191, 233, 247};

            std::array<u32, 7> actual{74, 155, 191, 9, 233, 247, 126};
            Sort(actual, actual.size());

            EXPECT(actual == expected);
        }

        SECTION("8") {
            std::array<u32, 8> expected{0, 17, 17, 35, 123, 126, 144, 214};

            std::array<u32, 8> actual{17, 144, 123, 214, 126, 17, 35, 0};
            Sort(actual, actual.size());

            EXPECT(actual == expected);
        }
    }
}

int main(int argc, char *argv[]) {
    return lest::run(specification, argc, argv, std::cerr);
}
