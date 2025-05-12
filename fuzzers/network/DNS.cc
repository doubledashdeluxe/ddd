#include <native/network/FakeDNS.hh>
#include <native/network/FakeUDPSocket.hh>
#include <portable/Bytes.hh>

#include <algorithm>
#include <cstring>

extern "C" int LLVMFuzzerTestOneInput(const u8 *data, size_t size) {
    std::vector<u8> chunk;
    FakeUDPSocket socket(chunk);
    FakeDNS dns(socket);
    while (size > 2) {
        u16 chunkSize = Bytes::ReadLE<u16>(data, 0);
        data += 2;
        size -= 2;

        chunk.clear();
        chunkSize = std::min<size_t>({chunkSize, size, 512});
        chunk.insert(chunk.begin(), data, data + chunkSize);
        data += chunkSize;
        size -= chunkSize;

        u32 address;
        dns.resolve("ddd.gg", address);
    }
    return 0;
}
