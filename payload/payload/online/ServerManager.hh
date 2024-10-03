#pragma once

#include "payload/StorageScanner.hh"

#include <common/Ring.hh>
#include <common/UniquePtr.hh>
#include <common/storage/Storage.hh>
#include <jsystem/JKRHeap.hh>

class ServerManager : public StorageScanner {
public:
    enum {
        MaxServerCount = 32,
    };

    struct Address {
        Array<u8, 4> address;
        u16 port;
    };

    class Server {
    public:
        Server(char *name, char *address, Array<u8, 32> publicKey);
        ~Server();

        const char *name() const;
        const char *address() const;
        Array<u8, 32> publicKey() const;

    private:
        UniquePtr<char[]> m_name;
        UniquePtr<char[]> m_address;
        Array<u8, 32> m_publicKey;
    };

    void start();

    u32 serverCount() const;
    const Server &server(u32 index) const;

    static void Init();
    static ServerManager *Instance();

private:
    struct ServerINI {
        Array<UniquePtr<char[]>, KartLocale::Language::Count> localizedNames;
        UniquePtr<char[]> fallbackName;
        UniquePtr<char[]> address;
        UniquePtr<char[]> publicKey;
    };

    ServerManager();

    void process() override;

    void addServers(Array<char, 256> &path, Storage::NodeInfo &nodeInfo);
    void addServer(const Array<char, 256> &path);
    void sortServersByName();

    static int HandleServerINI(void *user, const char *section, const char *name,
            const char *value);
    static bool CompareServersByName(const UniquePtr<Server> &a, const UniquePtr<Server> &b);

    JKRHeap *m_heap;
    Ring<UniquePtr<Server>, MaxServerCount> m_servers;

    static ServerManager *s_instance;
};
