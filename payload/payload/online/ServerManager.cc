#include "ServerManager.hh"

#include "payload/INIFileReader.hh"

#include <cube/Arena.hh>
#include <cube/Log.hh>
#include <portable/UTF8.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
#include <strings.h>
}

ServerManager::Server::Server(Array<char, 32> name, Array<char, 32> address,
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

u32 ServerManager::serverCount() const {
    return m_servers.count();
}

const ServerManager::Server &ServerManager::server(u32 index) const {
    return m_servers[index];
}

void ServerManager::Init() {
    s_instance = new (MEM1Arena::Instance(), 0x4) ServerManager;
}

ServerManager *ServerManager::Instance() {
    return s_instance;
}

ServerManager::ServerManager() {
    StorageScanner *param = this;
    OSCreateThread(&m_thread, Run, param, m_stack.values() + m_stack.count(), m_stack.count(), 27,
            0);
}

OSThread &ServerManager::thread() {
    return m_thread;
}

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
    Array<u8, 32> publicKey(0x0);
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

    Array<char, 32> name;
    snprintf(name.values(), name.count(), "%s", nameField.values());
    Array<char, 32> address;
    snprintf(address.values(), address.count(), "%s", addressField.values());

    DEBUG("Adding server %s...", path.values());
    Server server(name, address, publicKey);
    m_servers.pushBack(server);
}

void ServerManager::sortServersByName() {
    Sort(m_servers, m_servers.count(), CompareServersByName);
}

bool ServerManager::CompareServersByName(const Server &a, const Server &b) {
    const char *na = a.name(), *nb = b.name();
    return UTF8::CaseCompare(na, nb) <= 0;
}

ServerManager *ServerManager::s_instance = nullptr;
