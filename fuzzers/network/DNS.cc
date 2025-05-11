#include <native/network/FakeDNS.hh>
#include <native/network/FakeUDPSocket.hh>

#include <cstring>

extern "C" int LLVMFuzzerTestOneInput(const u8 *data, size_t size) {
    std::vector<u8> socketData;
    FakeUDPSocket socket(socketData);
    FakeDNS dns(socket);
    while (size > 0) {
        socketData.resize(std::min<size_t>(size, 512));
        memcpy(socketData.data(), data, socketData.size());
        u32 address;
        dns.resolve("ddd.gg", address);
        data += socketData.size();
        size -= socketData.size();
    }
    return 0;
}
