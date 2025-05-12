#pragma once

#include "payload/StorageScanner.hh"

#include <cube/storage/Storage.hh>
#include <portable/online/ServerManager.hh>

class CubeServerManager
    : public ServerManager
    , public StorageScanner {
public:
    bool isLocked() override;
    bool lock() override;
    void unlock() override;

    static void Init();
    static CubeServerManager *Instance();

private:
    struct ServerINI {
        Array<Array<char, INIReader::FieldSize>, KartLocale::Language::Count> localizedNames;
        Array<char, INIReader::FieldSize> fallbackName;
        Array<char, INIReader::FieldSize> address;
        Array<char, INIReader::FieldSize> publicKey;
    };

    CubeServerManager();

    OSThread &thread() override;
    void process() override;

    void addServers(Array<char, 256> &path, Storage::NodeInfo &nodeInfo);
    void addServer(const Array<char, 256> &path);
    void sortServersByName();

    static bool CompareServersByName(const Server &a, const Server &b);

    Ring<Server, MaxServerCount> m_servers;
    Array<u8, 16 * 1024> m_stack;
    OSThread m_thread;

    static CubeServerManager *s_instance;
};
