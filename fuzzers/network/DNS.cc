#include <native/network/FakeDNS.hh>

extern "C" int LLVMFuzzerTestOneInput(const u8 *data, size_t size) {
    while (size > 0) {
        std::vector<u8> dnsData(data, data + std::min<size_t>(size, 512));
        FakeDNS dns(dnsData);
        u32 address;
        dns.resolve("ddd.gg", address);
        data += dnsData.size();
        size -= dnsData.size();
    }
    return 0;
}
