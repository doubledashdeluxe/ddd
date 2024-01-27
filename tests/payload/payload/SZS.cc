#include <lest.hpp>
#include <payload/SZS.hh>

static lest::tests specification;

#define CASE(name) lest_CASE(specification, name)

CASE("SZS") {
    SETUP("Incomplete header") {
        std::array<u8, 0x5> compressed{0x59, 0x61, 0x7a, 0x30, 0x00};

        SECTION("CheckMagic") {
            EXPECT(!SZS::CheckMagic(compressed.data(), compressed.size()));
        }

        SECTION("GetUncompressedSize") {
            u32 uncompressedSize;
            EXPECT(!SZS::GetUncompressedSize(compressed.data(), compressed.size(),
                    uncompressedSize));
        }

        SECTION("Uncompress") {
            std::array<u8, 0> uncompressed;
            EXPECT(!SZS::Uncompress(compressed.data(), compressed.size(), uncompressed.data(),
                    uncompressed.size()));
        }
    }

    SETUP("Wrong magic") {
        std::array<u8, 0x15> compressed{0x59, 0x61, 0x79, 0x30, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x11, 0x70, 0x01};

        SECTION("CheckMagic") {
            EXPECT(!SZS::CheckMagic(compressed.data(), compressed.size()));
        }

        SECTION("GetUncompressedSize") {
            u32 uncompressedSize;
            EXPECT(!SZS::GetUncompressedSize(compressed.data(), compressed.size(),
                    uncompressedSize));
        }

        SECTION("Uncompress") {
            std::array<u8, 0> uncompressed;
            EXPECT(!SZS::Uncompress(compressed.data(), compressed.size(), uncompressed.data(),
                    uncompressed.size()));
        }
    }

    SETUP("Correct file") {
        std::array<u8, 0x15> compressed{0x59, 0x61, 0x7a, 0x30, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x11, 0x70, 0x01};

        SECTION("CheckMagic") {
            EXPECT(SZS::CheckMagic(compressed.data(), compressed.size()));
        }

        SECTION("GetUncompressedSize") {
            u32 uncompressedSize;
            EXPECT(SZS::GetUncompressedSize(compressed.data(), compressed.size(),
                    uncompressedSize));
            EXPECT(uncompressedSize == 0xb);
        }

        SECTION("Uncompress") {
            std::array<u8, 0xb> uncompressed;
            EXPECT(SZS::Uncompress(compressed.data(), compressed.size(), uncompressed.data(),
                    uncompressed.size()));
            for (u32 i = 0; i < uncompressed.size(); i++) {
                EXPECT(uncompressed[i] == 0x11 * (i % 0x2));
            }
        }
    }
}

int main(int argc, char *argv[]) {
    return lest::run(specification, argc, argv, std::cerr);
}
