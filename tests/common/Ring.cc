#include <common/Ring.hh>
#define SNITCH_IMPLEMENTATION
#include <snitch/snitch_all.hpp>

TEST_CASE("Ring") {
    SECTION("Empty") {
        Ring<u32, 8> ring;

        SECTION("empty") {
            CHECK(ring.empty());
        }

        SECTION("full") {
            CHECK_FALSE(ring.full());
        }

        SECTION("count") {
            CHECK(ring.count() == 0);
        }

        SECTION("front") {
            CHECK(ring.front() == nullptr);
        }

        SECTION("back") {
            CHECK(ring.back() == nullptr);
        }

        SECTION("pushFront") {
            CHECK(ring.pushFront(96));
            CHECK(*ring.front() == 96);
        }

        SECTION("pushBack") {
            CHECK(ring.pushBack(168));
            CHECK(*ring.back() == 168);
        }

        SECTION("popFront") {
            CHECK_FALSE(ring.popFront());
        }

        SECTION("popBack") {
            CHECK_FALSE(ring.popBack());
        }

        SECTION("reset") {
            ring.reset();
            CHECK(ring.empty());
        }
    }

    SECTION("Full") {
        Ring<u32, 8> ring;
        ring.pushBack(231);
        ring.pushBack(67);
        ring.pushBack(102);
        ring.pushBack(207);
        ring.pushBack(211);
        ring.pushBack(56);
        ring.pushBack(84);
        ring.pushBack(220);

        SECTION("empty") {
            CHECK_FALSE(ring.empty());
        }

        SECTION("full") {
            CHECK(ring.full());
        }

        SECTION("count") {
            CHECK(ring.count() == 8);
        }

        SECTION("front") {
            CHECK(*ring.front() == 231);
        }

        SECTION("back") {
            CHECK(*ring.back() == 220);
        }

        SECTION("operator[]") {
            CHECK(ring[6] == 84);
        }

        SECTION("pushFront") {
            CHECK_FALSE(ring.pushFront(100));
        }

        SECTION("pushBack") {
            CHECK_FALSE(ring.pushBack(106));
        }

        SECTION("popFront") {
            CHECK(ring.popFront());
            CHECK(*ring.front() == 67);
        }

        SECTION("popBack") {
            CHECK(ring.popBack());
            CHECK(*ring.back() == 84);
        }

        SECTION("swapRemoveFront") {
            ring.swapRemoveFront(5);
            CHECK(*ring.front() == 67);
            CHECK(ring[4] == 231);
        }

        SECTION("swapRemoveBack") {
            ring.swapRemoveBack(5);
            CHECK(*ring.back() == 84);
            CHECK(ring[5] == 220);
        }

        SECTION("reset") {
            ring.reset();
            CHECK(ring.empty());
        }
    }

    SECTION("Wrapped around") {
        Ring<u32, 8> ring;
        for (size_t i = 0; i < 6; i++) {
            ring.pushBack(115);
            ring.popFront();
        }
        ring.pushBack(70);
        ring.pushBack(32);
        ring.pushBack(123);

        SECTION("empty") {
            CHECK_FALSE(ring.empty());
        }

        SECTION("full") {
            CHECK_FALSE(ring.full());
        }

        SECTION("count") {
            CHECK(ring.count() == 3);
        }

        SECTION("front") {
            CHECK(*ring.front() == 70);
        }

        SECTION("back") {
            CHECK(*ring.back() == 123);
        }

        SECTION("operator[]") {
            CHECK(ring[1] == 32);
        }

        SECTION("pushFront") {
            CHECK(ring.pushFront(111));
            CHECK(*ring.front() == 111);
        }

        SECTION("pushBack") {
            CHECK(ring.pushBack(104));
            CHECK(*ring.back() == 104);
        }

        SECTION("popFront") {
            CHECK(ring.popFront());
            CHECK(*ring.front() == 32);
        }

        SECTION("popBack") {
            CHECK(ring.popBack());
            CHECK(*ring.back() == 32);
        }

        SECTION("swapRemoveFront") {
            ring.swapRemoveFront(1);
            CHECK(*ring.front() == 70);
        }

        SECTION("swapRemoveBack") {
            ring.swapRemoveBack(1);
            CHECK(*ring.back() == 123);
        }

        SECTION("reset") {
            ring.reset();
            CHECK(ring.empty());
        }
    }
}
