#include <native/network/DNSFakeUDPSocket.hh>
#include <native/network/FakeDNS.hh>
#include <snitch/snitch_all.hpp>

#include <map>
#include <string>
#include <vector>

TEST_CASE("DNS") {
    u32 expectedAddress = 192 << 24 | 0 << 16 | 2 << 8 | 0 << 0;
    std::map<std::vector<std::string>, u32> servers;
    servers[{"ddd", "gg"}] = expectedAddress;
    DNSFakeUDPSocket socket(servers);
    FakeDNS dns(socket);
    u32 actualAddress;
    CHECK_FALSE(dns.resolve("ddd.gg", actualAddress));
    CHECK(dns.resolve("ddd.gg", actualAddress));
    CHECK(actualAddress == expectedAddress);
    CHECK_FALSE(dns.resolve("doubledashde.luxe", actualAddress));
    CHECK_FALSE(dns.resolve("doubledashde.luxe", actualAddress));
}
