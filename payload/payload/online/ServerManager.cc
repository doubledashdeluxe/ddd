#include "ServerManager.hh"

#include "payload/FileLoader.hh"

#include <common/Arena.hh>
#include <common/Log.hh>
#include <jsystem/JKRExpHeap.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
#include <strings.h>
}

ServerManager::Server::Server(Array<char, INIFieldSize> name, Array<char, INIFieldSize> address,
        Array<u8, 32> publicKey)
    : m_name(name), m_address(address), m_publicKey(publicKey) {}

ServerManager::Server::~Server() {}

const char *ServerManager::Server::name() const {
    return m_name.values();
}

const char *ServerManager::Server::address() const {
    return m_address.values();
}

Array<u8, 32> ServerManager::Server::publicKey() const {
    return m_publicKey;
}

void ServerManager::start() {
    size_t heapSize = 0x80000;
    void *heapPtr = MEM2Arena::Instance()->alloc(heapSize, 0x4);
    m_heap = JKRExpHeap::Create(heapPtr, heapSize, JKRHeap::GetRootHeap(), false);
    StorageScanner::start();
}

u32 ServerManager::serverCount() const {
    return m_servers.count();
}

const ServerManager::Server &ServerManager::server(u32 index) const {
    return *m_servers[index];
}

void ServerManager::Init() {
    s_instance = new (MEM2Arena::Instance(), 0x4) ServerManager;
}

ServerManager *ServerManager::Instance() {
    return s_instance;
}

ServerManager::ServerManager() : StorageScanner(27) {}

void ServerManager::process() {
    m_servers.reset();
    Array<char, 256> path;
    snprintf(path.values(), path.count(), "main:/ddd/servers");
    Storage::CreateDir(path.values(), Storage::Mode::WriteAlways);
    Storage::NodeInfo nodeInfo;
    addServers(path, nodeInfo);
    sortServersByName();
}

void ServerManager::addServers(Array<char, 256> &path, Storage::NodeInfo &nodeInfo) {
    u32 length = strlen(path.values());
    for (Storage::DirHandle dir(path.values()); dir.read(nodeInfo);) {
        snprintf(path.values() + length, path.count() - length, "/%s", nodeInfo.name.values());
        if (nodeInfo.type == Storage::NodeType::Dir) {
            addServers(path, nodeInfo);
        } else {
            addServer(path);
        }
    }
    path[length] = '\0';
}

void ServerManager::addServer(const Array<char, 256> &path) {
    INI::Stream iniStream;
    iniStream.ini.reset(
            reinterpret_cast<u8 *>(FileLoader::Load(path.values(), m_heap, &iniStream.iniSize)));
    if (!iniStream.ini.get()) {
        return;
    }
    iniStream.iniOffset = 0x0;

    ServerINI serverINI;
    Array<INI::Field, 3> iniFields;
    iniFields[0] = (INI::Field){"servername", &serverINI.fallbackName};
    iniFields[1] = (INI::Field){"address", &serverINI.address};
    iniFields[2] = (INI::Field){"publickey", &serverINI.publicKey};
    Array<INI::LocalizedField, 1> localizedINIFields;
    localizedINIFields[0] = (INI::LocalizedField){"servername_", &serverINI.localizedNames};

    INI ini(iniStream, iniFields.count(), iniFields.values(), localizedINIFields.count(),
            localizedINIFields.values());
    if (!ini.read()) {
        return;
    }

    Array<char, INIFieldSize> &name =
            getLocalizedEntry(serverINI.localizedNames, serverINI.fallbackName);
    if (strlen(name.values()) == 0) {
        return;
    }
    Array<char, INIFieldSize> &address = serverINI.address;
    if (strlen(address.values()) == 0) {
        return;
    }
    Array<u8, 32> publicKey(0x0);
    if (strlen(serverINI.publicKey.values()) != publicKey.count() * 2) {
        return;
    }
    for (u32 i = 0; i < publicKey.count() * 2; i++) {
        publicKey[i / 2] <<= 4;
        char c = serverINI.publicKey[i];
        if (c >= '0' && c <= '9') {
            publicKey[i / 2] |= c - '0';
        } else if (c >= 'A' && c <= 'Z') {
            publicKey[i / 2] |= c - 'A' + 0xa;
        } else if (c >= 'a' && c <= 'z') {
            publicKey[i / 2] |= c - 'a' + 0xa;
        } else {
            return;
        }
    }

    DEBUG("Adding server %s...", path.values());
    m_servers.pushBack();
    Server *server = new (m_heap, 0x4) Server(name, address, publicKey);
    m_servers.back()->reset(server);
}

void ServerManager::sortServersByName() {
    Sort(m_servers, m_servers.count(), CompareServersByName);
}

bool ServerManager::CompareServersByName(const UniquePtr<Server> &a, const UniquePtr<Server> &b) {
    return strcasecmp(a->name(), b->name()) <= 0;
}

ServerManager *ServerManager::s_instance = nullptr;
