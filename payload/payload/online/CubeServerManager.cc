#include "CubeServerManager.hh"

#include "payload/INIFileReader.hh"

#include <cube/Arena.hh>
#include <formats/Online.hh>
#include <formats/Version.hh>
#include <portable/Log.hh>
#include <portable/UTF8.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
}

bool CubeServerManager::isLocked() {
    return StorageScanner::isLocked();
}

bool CubeServerManager::lock() {
    return StorageScanner::lock();
}

void CubeServerManager::unlock() {
    StorageScanner::unlock();
}

void CubeServerManager::Init() {
    s_instance = new (MEM1Arena::Instance(), 0x4) CubeServerManager;
}

CubeServerManager *CubeServerManager::Instance() {
    return s_instance;
}

CubeServerManager::CubeServerManager() : ServerManager(m_servers) {
    StorageScanner *param = this;
    OSCreateThread(&m_thread, Run, param, m_stack.values() + m_stack.count(), m_stack.count(), 27,
            0);
}

OSThread &CubeServerManager::thread() {
    return m_thread;
}

void CubeServerManager::process() {
    m_servers.reset();
    addDefaultServers();
    Array<char, 256> path;
    snprintf(path.values(), path.count(), "main:/ddd/servers");
    Storage::CreateDir(path.values(), Storage::Mode::WriteAlways);
    Storage::NodeInfo nodeInfo;
    addCustomServers(path, nodeInfo);
    sortServersByName();
}

void CubeServerManager::addDefaultServers() {
    bool isDev = strstr(Version, "dev");
    bool isBeta = strstr(Version, "beta");
    if (isDev) {
        u8 publicKey[32] = {0xec, 0x54, 0x34, 0xbe, 0x35, 0x1d, 0xab, 0xf2, 0x81, 0x8c, 0xd2, 0x35,
                0x5a, 0xe2, 0xe3, 0xa5, 0x8c, 0x79, 0xa9, 0xc4, 0x99, 0xff, 0x4a, 0x5b, 0x75, 0xcb,
                0x7b, 0xb9, 0xa5, 0xcf, 0xbd, 0x21};
        addDefaultServer("Dev Server", "dev.ddd.gg", publicKey);
    }
    if (isDev || isBeta) {
        u8 publicKey[32] = {0x82, 0x7d, 0xbb, 0x08, 0x77, 0x2b, 0x67, 0x61, 0x4c, 0x1c, 0xaf, 0xdc,
                0x58, 0x7e, 0x91, 0xde, 0x7e, 0x13, 0x1c, 0xf4, 0xe7, 0xd9, 0x9e, 0xe3, 0xdf, 0x71,
                0x6b, 0xd3, 0x80, 0x39, 0xea, 0x25};
        addDefaultServer("Beta Server", "beta.ddd.gg", publicKey);
    }
}

void CubeServerManager::addDefaultServer(const char *name, const char *address,
        const u8 (&publicKey)[32]) {
    DEBUG("Adding default server %s...", name);
    ServerName serverName;
    snprintf(serverName.values(), serverName.count(), "%s", name);
    Array<char, 80> serverAddress;
    snprintf(serverAddress.values(), serverAddress.count(), "%s", address);
    Server server(serverName, serverAddress, Optional<u16>(), PublicKey(publicKey));
    m_servers.pushBack(server);
}

void CubeServerManager::addCustomServers(Array<char, 256> &path, Storage::NodeInfo &nodeInfo) {
    u32 length = strlen(path.values());
    for (Storage::DirHandle dir(path.values()); dir.read(nodeInfo);) {
        snprintf(path.values() + length, path.count() - length, "/%s", nodeInfo.name.values());
        if (nodeInfo.type == Storage::NodeType::Dir) {
            addCustomServers(path, nodeInfo);
        } else {
            addCustomServer(path);
        }
    }
    path[length] = '\0';
}

void CubeServerManager::addCustomServer(const Array<char, 256> &path) {
    ServerINI serverINI;
    Array<INIReader::Field, 3> iniFields;
    iniFields[0] = (INIReader::Field){"servername", &serverINI.fallbackName};
    iniFields[1] = (INIReader::Field){"address", &serverINI.address};
    iniFields[2] = (INIReader::Field){"publickey", &serverINI.publicKey};
    Array<INIReader::LocalizedField, 1> localizedINIFields;
    localizedINIFields[0] = (INIReader::LocalizedField){"servername_", &serverINI.localizedNames};
    if (!INIFileReader::Read(iniFields.count(), iniFields.values(), localizedINIFields.count(),
                localizedINIFields.values(), path.values())) {
        return;
    }

    Array<char, INIReader::FieldSize> &nameField =
            getLocalizedEntry(serverINI.localizedNames, serverINI.fallbackName);
    if (strlen(nameField.values()) == 0) {
        return;
    }
    Array<char, INIReader::FieldSize> &addressField = serverINI.address;
    if (strlen(addressField.values()) == 0) {
        return;
    }
    Array<char, INIReader::FieldSize> &publicKeyField = serverINI.publicKey;
    PublicKey publicKey(0x00);
    if (strlen(publicKeyField.values()) != publicKey.count() * 2) {
        return;
    }
    for (u32 i = 0; i < publicKey.count() * 2; i++) {
        publicKey[i / 2] <<= 4;
        char c = publicKeyField[i];
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

    ServerName name;
    snprintf(name.values(), name.count(), "%s", nameField.values());
    Array<char, 80> address;
    Optional<u16> port;
    const char *tail = strrchr(addressField.values(), ':');
    if (tail && sscanf(tail, ":%hu", &port.emplace()) == 1) {
        snprintf(address.values(), address.count(), "%.*s",
                static_cast<int>(tail - addressField.values()), addressField.values());
    } else {
        address = addressField;
        port.reset();
    }

    for (u32 i = 0; i < m_servers.count(); i++) {
        if (!strcmp(m_servers[i].address().values(), address.values()) &&
                m_servers[i].publicKey() == publicKey) {
            return;
        }
    }

    DEBUG("Adding custom server %s...", path.values());
    Server server(name, address, port, publicKey);
    m_servers.pushBack(server);
}

void CubeServerManager::sortServersByName() {
    Sort(m_servers, m_servers.count(), CompareServersByName);
}

bool CubeServerManager::CompareServersByName(const Server &a, const Server &b) {
    const char *na = a.name().values(), *nb = b.name().values();
    return UTF8::CaseCompare(na, nb) <= 0;
}

CubeServerManager *CubeServerManager::s_instance = nullptr;
