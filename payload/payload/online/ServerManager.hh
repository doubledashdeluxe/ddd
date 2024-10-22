#pragma once

#include "payload/StorageScanner.hh"

#include <common/Ring.hh>
#include <common/storage/Storage.hh>

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
        Server(Array<char, 32> name, Array<char, 32> address, Array<u8, 32> publicKey);
        ~Server();

        const char *name() const;
        const char *address() const;
        Array<u8, 32> publicKey() const;

    private:
        Array<char, 32> m_name;
        Array<char, 32> m_address;
        Array<u8, 32> m_publicKey;
    };

    u32 serverCount() const;
    const Server &server(u32 index) const;

    static void Init();
    static ServerManager *Instance();

private:
    struct ServerINI {
        Array<Array<char, INIReader::FieldSize>, KartLocale::Language::Count> localizedNames;
        Array<char, INIReader::FieldSize> fallbackName;
        Array<char, INIReader::FieldSize> address;
        Array<char, INIReader::FieldSize> publicKey;
    };

    ServerManager();

    OSThread &thread() override;
    void process() override;

    void addServers(Array<char, 256> &path, Storage::NodeInfo &nodeInfo);
    void addServer(const Array<char, 256> &path);
    void sortServersByName();

    static bool CompareServersByName(const Server &a, const Server &b);

    Array<u8, 16 * 1024> m_stack;
    OSThread m_thread;
    Ring<Server, MaxServerCount> m_servers;

    static ServerManager *s_instance;
};
