#include <portable/Ring.hh>
#include <snitch/snitch_all.hpp>

TEST_CASE("Ring") {
    SECTION("Empty") {
        Ring<u32, 7> ring;

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

        SECTION("emplace") {
            CHECK(*ring.emplaceBack() = 168);
            CHECK(*ring.emplaceFront() = 96);
            CHECK(*ring.back() == 168);
            CHECK(*ring.front() == 96);
        }

        SECTION("push") {
            CHECK(ring.pushBack(168));
            CHECK(ring.pushFront(96));
            CHECK(*ring.back() == 168);
            CHECK(*ring.front() == 96);
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
        Ring<u32, 7> ring;
        ring.pushBack(231);
        ring.pushBack(67);
        ring.pushBack(102);
        ring.pushBack(207);
        ring.pushBack(211);
        ring.pushBack(56);
        ring.pushBack(84);

        SECTION("empty") {
            CHECK_FALSE(ring.empty());
        }

        SECTION("full") {
            CHECK(ring.full());
        }

        SECTION("count") {
            CHECK(ring.count() == 7);
        }

        SECTION("front") {
            CHECK(*ring.front() == 231);
        }

        SECTION("back") {
            CHECK(*ring.back() == 84);
        }

        SECTION("operator[]") {
            CHECK(ring[5] == 56);
        }

        SECTION("emplaceFront") {
            CHECK_FALSE(ring.emplaceFront());
        }

        SECTION("emplaceBack") {
            CHECK_FALSE(ring.emplaceBack());
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
            CHECK(*ring.back() == 56);
        }

        SECTION("swapRemoveFront") {
            ring.swapRemoveFront(5);
            CHECK(*ring.front() == 67);
            CHECK(ring[4] == 231);
        }

        SECTION("swapRemoveBack") {
            ring.swapRemoveBack(5);
            CHECK(*ring.back() == 84);
            CHECK(ring[5] == 84);
        }

        SECTION("reset") {
            ring.reset();
            CHECK(ring.empty());
        }
    }

    SECTION("Wrapped around") {
        Ring<u32, 7> ring;
        for (size_t i = 0; i < 5; i++) {
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

        SECTION("emplace") {
            CHECK(*ring.emplaceFront() = 111);
            CHECK(*ring.emplaceBack() = 104);
            CHECK(*ring.front() == 111);
            CHECK(*ring.back() == 104);
        }

        SECTION("push") {
            CHECK(ring.pushFront(111));
            CHECK(ring.pushBack(104));
            CHECK(*ring.front() == 111);
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
