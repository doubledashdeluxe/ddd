#include <portable/Bytes.hh>
#include <snitch/snitch_all.hpp>

TEST_CASE("Bytes") {
    std::array<u8, 4> buffer{0x12, 0x34, 0x56, 0x78};

    SECTION("ReadBE") {
        CHECK(Bytes::ReadBE<u8>(buffer.data(), 0) == 0x12);
        CHECK(Bytes::ReadBE<u8>(buffer.data(), 1) == 0x34);
        CHECK(Bytes::ReadBE<u8>(buffer.data(), 2) == 0x56);
        CHECK(Bytes::ReadBE<u8>(buffer.data(), 3) == 0x78);
        CHECK(Bytes::ReadBE<u16>(buffer.data(), 0) == 0x1234);
        CHECK(Bytes::ReadBE<u16>(buffer.data(), 1) == 0x3456);
        CHECK(Bytes::ReadBE<u16>(buffer.data(), 2) == 0x5678);
        CHECK(Bytes::ReadBE<u32>(buffer.data(), 0) == 0x12345678);
    }

    SECTION("ReadLE") {
        CHECK(Bytes::ReadLE<u8>(buffer.data(), 0) == 0x12);
        CHECK(Bytes::ReadLE<u8>(buffer.data(), 1) == 0x34);
        CHECK(Bytes::ReadLE<u8>(buffer.data(), 2) == 0x56);
        CHECK(Bytes::ReadLE<u8>(buffer.data(), 3) == 0x78);
        CHECK(Bytes::ReadLE<u16>(buffer.data(), 0) == 0x3412);
        CHECK(Bytes::ReadLE<u16>(buffer.data(), 1) == 0x5634);
        CHECK(Bytes::ReadLE<u16>(buffer.data(), 2) == 0x7856);
        CHECK(Bytes::ReadLE<u32>(buffer.data(), 0) == 0x78563412);
    }

    SECTION("WriteBE") {
        Bytes::WriteBE<u16>(buffer.data(), 1, 0xabcd);

        std::array<u8, 4> expected{0x12, 0xab, 0xcd, 0x78};
        CHECK(buffer == expected);
    }

    SECTION("WriteLE") {
        Bytes::WriteLE<u16>(buffer.data(), 1, 0xcdab);

        std::array<u8, 4> expected{0x12, 0xab, 0xcd, 0x78};
        CHECK(buffer == expected);
    }
}
