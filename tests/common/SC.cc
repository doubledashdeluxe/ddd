#include <common/SC.hh>
#include <snitch/snitch_all.hpp>

#include <cstring>

TEST_CASE("SC") {
    SECTION("Invalid header") {
        Array<u8, 0x4000> buffer(0);

        SC sc(buffer);

        SECTION("ok") {
            CHECK_FALSE(sc.ok());
        }

        SECTION("get") {
            u32 value;
            CHECK_FALSE(sc.get("My value", value));
            CHECK_FALSE(sc.get("My other value", value));
        }
    }

    SECTION("Invalid contents") {
        Array<u8, 0x4000> buffer(0);
        memcpy(buffer.values() + 0x0, "SCv0", strlen("SCv0"));

        SC sc(buffer);

        SECTION("ok") {
            CHECK(sc.ok());
        }

        SECTION("get") {
            u32 value;
            CHECK_FALSE(sc.get("My value", value));
            CHECK_FALSE(sc.get("My other value", value));
        }
    }

    SECTION("Valid") {
        Array<u8, 0x4000> buffer(0);
        memcpy(buffer.values() + 0x0, "SCv0", strlen("SCv0"));
        Bytes::WriteBE<u16>(buffer.values(), 0x4, 0x1);
        Bytes::WriteBE<u16>(buffer.values(), 0x6, 0x8);
        Bytes::WriteBE<u8>(buffer.values(), 0x8, 5 << 5 | (strlen("My value") - 1) << 0);
        memcpy(buffer.values() + 0x9, "My value", strlen("My value"));
        Bytes::WriteBE<u32>(buffer.values(), 0x9 + strlen("My value"), 42);

        SC sc(buffer);

        SECTION("ok") {
            CHECK(sc.ok());
        }

        SECTION("get") {
            u32 value;
            CHECK(sc.get("My value", value));
            CHECK(value == 42);
            CHECK_FALSE(sc.get("My other value", value));
        }
    }
}
