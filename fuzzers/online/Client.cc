extern "C" {
#include <monocypher/monocypher.h>
}
#include <native/online/FakeClient.hh>
#include <portable/Bytes.hh>

#include <algorithm>

extern "C" int LLVMFuzzerTestOneInput(const u8 *data, size_t size) {
    std::map<std::vector<std::string>, u32> dnsServers;
    dnsServers[{"test", "ddd", "gg"}] = 192 << 24 | 0 << 16 | 2 << 8 | 0 << 0;

    Array<u8, 32> clientK, serverK, serverPK;
    NativeRandom random;
    random.get(clientK.values(), clientK.count());
    random.get(serverK.values(), serverK.count());
    crypto_x25519_public_key(serverPK.values(), serverK.values());

    Array<char, 32> name;
    snprintf(name.values(), name.count(), "Test server");
    Array<char, 32> address;
    snprintf(address.values(), address.count(), "test.ddd.gg");
    ServerManager::Server server(name, address, serverPK);
    Ring<ServerManager::Server, ServerManager::MaxServerCount> servers;
    servers.pushBack(server);

    std::vector<u8> chunk;
    FakeClient client(dnsServers, serverK, chunk, servers, clientK);
    while (size > 2) {
        u16 chunkSize = Bytes::ReadLE<u16>(data, 0);
        data += 2;
        size -= 2;

        chunk.clear();
        chunkSize = std::min<size_t>({chunkSize, size, 512});
        chunk.insert(chunk.begin(), data, data + chunkSize);
        data += chunkSize;
        size -= chunkSize;

        client.read();
        client.write();
    }
    return 0;
}
