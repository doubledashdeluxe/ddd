#include <native/network/DNSFakeUDPSocket.hh>
#include <native/network/FakeDNS.hh>
#include <snitch/snitch_all.hpp>

TEST_CASE("DNS") {
    u32 expectedAddress = 192 << 24 | 0 << 16 | 2 << 8 | 0 << 0;
    DNSFakeUDPSocket::AEntries aEntries;
    aEntries["ddd.gg"] = expectedAddress;

    DNSFakeUDPSocket::Target expectedTarget = {3549, "ddd.ddd.gg"};
    DNSFakeUDPSocket::SRVEntries srvEntries;
    srvEntries["_ddd._udp.ddd.gg"] = expectedTarget;

    DNSFakeUDPSocket socket(aEntries, srvEntries);
    FakeDNS dns(socket);
    DNS::Resolvers resolvers(0);

    Optional<u32> actualAddress;
    CHECK_FALSE(dns.resolveA(resolvers, "ddd.gg", actualAddress));
    CHECK(dns.resolveA(resolvers, "ddd.gg", actualAddress));
    CHECK(actualAddress == expectedAddress);
    CHECK_FALSE(dns.resolveA(resolvers, "doubledashde.luxe", actualAddress));
    CHECK(dns.resolveA(resolvers, "doubledashde.luxe", actualAddress));
    CHECK_FALSE(actualAddress);

    Optional<DNS::Target> actualTarget;
    CHECK_FALSE(dns.resolveSRV(resolvers, "_ddd._udp.ddd.gg", actualTarget));
    CHECK(dns.resolveSRV(resolvers, "_ddd._udp.ddd.gg", actualTarget));
    CHECK(actualTarget);
    CHECK(actualTarget->port == expectedTarget.port);
    CHECK(actualTarget->name.values() == expectedTarget.name);
    CHECK_FALSE(dns.resolveSRV(resolvers, "_ddd._udp.doubledashde.luxe", actualTarget));
    CHECK(dns.resolveSRV(resolvers, "_ddd._udp.doubledashde.luxe", actualTarget));
    CHECK_FALSE(actualTarget);
}
