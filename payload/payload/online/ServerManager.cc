#include "ServerManager.hh"

#include <common/Arena.hh>
#include <common/DCache.hh>
#include <common/Log.hh>
extern "C" {
#include <inih/ini.h>
}
#include <jsystem/JKRExpHeap.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
#include <strings.h>
}

ServerManager::Server::Server(char *name, char *address, Array<u8, 32> publicKey, u8 *nameImage)
    : m_name(name), m_address(address), m_publicKey(publicKey), m_nameImage(nameImage) {}

ServerManager::Server::~Server() {}

const char *ServerManager::Server::name() const {
    return m_name.get();
}

const char *ServerManager::Server::address() const {
    return m_address.get();
}

Array<u8, 32> ServerManager::Server::publicKey() const {
    return m_publicKey;
}

void *ServerManager::Server::nameImage() const {
    return m_nameImage.get();
}

void ServerManager::start() {
    size_t heapSize = 0x80000;
    void *heap = MEM2Arena::Instance()->alloc(heapSize, 0x4);
    m_heap = JKRExpHeap::Create(heap, heapSize, JKRHeap::GetRootHeap(), false);
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
    addServers(path);
    sortServersByName();
}

void ServerManager::addServers(Array<char, 256> &path) {
    u32 length = strlen(path.values());
    Storage::NodeInfo nodeInfo;
    for (Storage::DirHandle dir(path.values()); dir.read(nodeInfo);) {
        snprintf(path.values() + length, path.count() - length, "/%s", nodeInfo.name.values());
        if (nodeInfo.type == Storage::NodeType::Dir) {
            addServers(path);
        } else {
            addServer(path);
        }
    }
    path[length] = '\0';
}

void ServerManager::addServer(const Array<char, 256> &path) {
    bool ok;
    ZIPFile zipFile(path.values(), ok);
    if (!ok) {
        return;
    }

    ZIPFile::CDNode cdNode;
    ZIPFile::LocalNode localNode;
    INIStream iniStream;
    iniStream.ini.reset(reinterpret_cast<u8 *>(
            zipFile.readFile("serverinfo.ini", m_heap, -0x4, cdNode, localNode)));
    if (!iniStream.ini.get()) {
        return;
    }
    iniStream.iniSize = cdNode.uncompressedSize;
    iniStream.iniOffset = 0x0;

    Array<char, 128> prefix;
    u32 prefixLength = strlen(cdNode.path.get()) - strlen("serverinfo.ini");
    if (prefixLength > prefix.count()) {
        return;
    }
    prefix[prefixLength] = '\0';
    memcpy(prefix.values(), cdNode.path.get(), prefixLength);

    ServerINI serverINI;
    if (ini_parse_stream(ReadINI, &iniStream, HandleServerINI, &serverINI)) {
        return;
    }

    UniquePtr<char[]> name(
            getLocalizedEntry(serverINI.localizedNames, serverINI.fallbackName).release());
    if (!name.get()) {
        return;
    }
    UniquePtr<char[]> address(serverINI.address.release());
    if (!address.get()) {
        return;
    }
    Array<u8, 32> publicKey(0x0);
    if (!serverINI.publicKey.get() || strlen(serverINI.publicKey.get()) != publicKey.count() * 2) {
        return;
    }
    for (u32 i = 0; i < publicKey.count() * 2; i++) {
        publicKey[i / 2] <<= 4;
        char c = serverINI.publicKey.get()[i];
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

    Array<char, 256> nameImagePrefix;
    snprintf(nameImagePrefix.values(), nameImagePrefix.count(), "/%sserver_images/",
            prefix.values());
    u32 nameImageSize;
    UniquePtr<u8[]> nameImage(reinterpret_cast<u8 *>(loadLocalizedFile(zipFile,
            nameImagePrefix.values(), "/server_name.bti", m_heap, &nameImageSize)));
    if (!nameImage.get() || nameImageSize < 0x20) {
        return;
    }
    DCache::Flush(nameImage.get(), nameImageSize);

    DEBUG("Adding server %s...", path.values());
    m_servers.pushBack();
    Server *server =
            new (m_heap, 0x4) Server(name.release(), address.get(), publicKey, nameImage.release());
    m_servers.back()->reset(server);
}

void ServerManager::sortServersByName() {
    Sort(m_servers, m_servers.count(), CompareServersByName);
}

int ServerManager::HandleServerINI(void *user, const char *section, const char *name,
        const char *value) {
    if (strcmp(section, "Config")) {
        return 1;
    }

    ServerINI *serverINI = reinterpret_cast<ServerINI *>(user);
    Array<INIField, 3> iniFields;
    iniFields[0] = (INIField){"servername", &serverINI->fallbackName};
    iniFields[1] = (INIField){"address", &serverINI->address};
    iniFields[2] = (INIField){"publickey", &serverINI->publicKey};
    if (HandleINIFields(name, value, iniFields.count(), iniFields.values(), s_instance->m_heap)) {
        return 1;
    }

    Array<LocalizedINIField, 1> localizedINIFields;
    localizedINIFields[0] = (LocalizedINIField){"servername_", &serverINI->localizedNames};
    if (HandleLocalizedINIFields(name, value, localizedINIFields.count(),
                localizedINIFields.values(), s_instance->m_heap)) {
        return 1;
    }

    return 1;
}

bool ServerManager::CompareServersByName(const UniquePtr<Server> &a, const UniquePtr<Server> &b) {
    return strcasecmp(a->name(), b->name()) <= 0;
}

ServerManager *ServerManager::s_instance = nullptr;
