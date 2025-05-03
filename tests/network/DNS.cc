#include <native/network/FakeDNS.hh>
#include <native/network/FakeUDPSocket.hh>
#include <snitch/snitch_all.hpp>

TEST_CASE("DNS") {
    SECTION("Empty") {
        std::vector<u8> socketData;
        FakeUDPSocket socket(socketData);
        FakeDNS dns(socket);
        u32 address;
        CHECK_FALSE(dns.resolve("ddd.gg", address));
        CHECK_FALSE(dns.resolve("ddd.gg", address));
        CHECK_FALSE(dns.resolve("doubledashde.luxe", address));
        CHECK_FALSE(dns.resolve("doubledashde.luxe", address));
    }

    SECTION("Valid") {
        std::vector<u8> socketData{0x00, 0x00, 0x81, 0x80, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x00, 0x03, 0x64, 0x64, 0x64, 0x02, 0x67, 0x67, 0x00, 0x00, 0x01, 0x00, 0x01, 0xc0,
                0x0c, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x08, 0xe2, 0x00, 0x04, 0xc0, 0x00, 0x02,
                0x00};
        FakeUDPSocket socket(socketData);
        FakeDNS dns(socket);
        u32 address;
        CHECK_FALSE(dns.resolve("ddd.gg", address));
        CHECK(dns.resolve("ddd.gg", address));
        CHECK(address == (192 << 24 | 0 << 16 | 2 << 8 | 0 << 0));
        CHECK_FALSE(dns.resolve("doubledashde.luxe", address));
        CHECK_FALSE(dns.resolve("doubledashde.luxe", address));
    }
}
