#include <native/FakeSZSReader.hh>
#include <snitch/snitch_all.hpp>

#include <vector>

TEST_CASE("SZSReader") {
    SECTION("Incomplete header") {
        std::vector<u8> compressed{0x59, 0x61, 0x7a, 0x30, 0x00};

        SECTION("read") {
            FakeSZSReader reader(&compressed);
            CHECK_FALSE(reader.read());
        }
    }

    SECTION("Wrong magic") {
        std::vector<u8> compressed{0x59, 0x61, 0x79, 0x30, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x11, 0x70, 0x01};

        SECTION("read") {
            FakeSZSReader reader(&compressed);
            CHECK_FALSE(reader.read());
        }
    }

    SECTION("Correct file") {
        std::vector<u8> compressed{0x59, 0x61, 0x7a, 0x30, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x11, 0x70, 0x01};

        SECTION("read") {
            FakeSZSReader reader(&compressed);
            CHECK(reader.read());
            CHECK(reader.uncompressedSize() == 0xb);
            for (u32 i = 0; i < reader.uncompressedSize(); i++) {
                CHECK(reader.uncompressed()[i] == 0x11 * (i % 0x2));
            }
        }
    }
}
