#include <native/network/FakeDNS.hh>
#include <native/network/FakeUDPSocket.hh>

extern "C" int LLVMFuzzerTestOneInput(const u8 *data, size_t size) {
    while (size > 0) {
        std::vector<u8> socketData(data, data + std::min<size_t>(size, 512));
        FakeUDPSocket socket(socketData);
        FakeDNS dns(socket);
        u32 address;
        dns.resolve("ddd.gg", address);
        data += socketData.size();
        size -= socketData.size();
    }
    return 0;
}
