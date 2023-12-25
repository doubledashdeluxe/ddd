#include <common/Ring.hh>
#include <lest.hpp>

static lest::tests specification;

#define CASE(name) lest_CASE(specification, name)

CASE("Ring") {
    SETUP("Empty") {
        Ring<u32, 8> ring;

        SECTION("empty") {
            EXPECT(ring.empty());
        }

        SECTION("full") {
            EXPECT(!ring.full());
        }

        SECTION("count") {
            EXPECT(ring.count() == 0);
        }

        SECTION("front") {
            EXPECT(ring.front() == nullptr);
        }

        SECTION("back") {
            EXPECT(ring.back() == nullptr);
        }

        SECTION("pushFront") {
            EXPECT(ring.pushFront(96));
            EXPECT(*ring.front() == 96);
        }

        SECTION("pushBack") {
            EXPECT(ring.pushBack(168));
            EXPECT(*ring.back() == 168);
        }

        SECTION("popFront") {
            EXPECT(!ring.popFront());
        }

        SECTION("popBack") {
            EXPECT(!ring.popBack());
        }

        SECTION("reset") {
            ring.reset();
            EXPECT(ring.empty());
        }
    }

    SETUP("Full") {
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
            EXPECT(!ring.empty());
        }

        SECTION("full") {
            EXPECT(ring.full());
        }

        SECTION("count") {
            EXPECT(ring.count() == 8);
        }

        SECTION("front") {
            EXPECT(*ring.front() == 231);
        }

        SECTION("back") {
            EXPECT(*ring.back() == 220);
        }

        SECTION("operator[]") {
            EXPECT(ring[6] == 84);
        }

        SECTION("pushFront") {
            EXPECT(!ring.pushFront(100));
        }

        SECTION("pushBack") {
            EXPECT(!ring.pushBack(106));
        }

        SECTION("popFront") {
            EXPECT(ring.popFront());
            EXPECT(*ring.front() == 67);
        }

        SECTION("popBack") {
            EXPECT(ring.popBack());
            EXPECT(*ring.back() == 84);
        }

        SECTION("swapRemoveFront") {
            ring.swapRemoveFront(5);
            EXPECT(*ring.front() == 67);
            EXPECT(ring[4] == 231);
        }

        SECTION("swapRemoveBack") {
            ring.swapRemoveBack(5);
            EXPECT(*ring.back() == 84);
            EXPECT(ring[5] == 220);
        }

        SECTION("reset") {
            ring.reset();
            EXPECT(ring.empty());
        }
    }

    SETUP("Wrapped around") {
        Ring<u32, 8> ring;
        for (size_t i = 0; i < 6; i++) {
            ring.pushBack(115);
            ring.popFront();
        }
        ring.pushBack(70);
        ring.pushBack(32);
        ring.pushBack(123);

        SECTION("empty") {
            EXPECT(!ring.empty());
        }

        SECTION("full") {
            EXPECT(!ring.full());
        }

        SECTION("count") {
            EXPECT(ring.count() == 3);
        }

        SECTION("front") {
            EXPECT(*ring.front() == 70);
        }

        SECTION("back") {
            EXPECT(*ring.back() == 123);
        }

        SECTION("operator[]") {
            EXPECT(ring[1] == 32);
        }

        SECTION("pushFront") {
            EXPECT(ring.pushFront(111));
            EXPECT(*ring.front() == 111);
        }

        SECTION("pushBack") {
            EXPECT(ring.pushBack(104));
            EXPECT(*ring.back() == 104);
        }

        SECTION("popFront") {
            EXPECT(ring.popFront());
            EXPECT(*ring.front() == 32);
        }

        SECTION("popBack") {
            EXPECT(ring.popBack());
            EXPECT(*ring.back() == 32);
        }

        SECTION("swapRemoveFront") {
            ring.swapRemoveFront(1);
            EXPECT(*ring.front() == 70);
        }

        SECTION("swapRemoveBack") {
            ring.swapRemoveBack(1);
            EXPECT(*ring.back() == 123);
        }

        SECTION("reset") {
            ring.reset();
            EXPECT(ring.empty());
        }
    }
}

int main(int argc, char *argv[]) {
    return lest::run(specification, argc, argv, std::cerr);
}
