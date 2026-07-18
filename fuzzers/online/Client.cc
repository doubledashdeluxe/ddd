#include <formats/Online.hh>
extern "C" {
#include <monocypher/monocypher.h>
}
#include <native/online/FakeClient.hh>
#include <portable/Bytes.hh>

#include <algorithm>

extern "C" int LLVMFuzzerTestOneInput(const u8 *data, size_t size) {
    FakeClient::AEntries aEntries;
    aEntries["test.ddd.gg"] = 192 << 24 | 0 << 16 | 2 << 8 | 0 << 0;
    FakeClient::SRVEntries srvEntries;
    srvEntries["_ddd._udp.ddd.gg"] = {3549, "ddd.ddd.gg"};

    Key clientK, serverK;
    PublicKey serverPK;
    NativeRandom random;
    random.get(clientK.values(), clientK.count());
    random.get(serverK.values(), serverK.count());
    crypto_x25519_public_key(serverPK.values(), serverK.values());

    ServerName name;
    snprintf(name.values(), name.count(), "Test server");
    Array<char, 80> address;
    snprintf(address.values(), address.count(), "test.ddd.gg");
    ServerManager::Server server(name, address, {}, serverPK);
    Ring<ServerManager::Server, MaxServerCount> servers;
    servers.pushBack(server);

    std::vector<u8> chunk;
    FakeClient client(aEntries, srvEntries, serverK, chunk, servers, clientK);
    while (size > 2) {
        u16 chunkSize = Bytes::ReadLE<u16>(data, 0);
        data += 2;
        size -= 2;

        chunk.clear();
        chunkSize = std::min<size_t>({chunkSize, size, BufferSize});
        chunk.insert(chunk.begin(), data, data + chunkSize);
        data += chunkSize;
        size -= chunkSize;

        client.read();
        client.write();
    }
    return 0;
}
