#include "ServerManager.hh"

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

ServerManager::ServerManager(const Ring<Server, MaxServerCount> &servers) : m_servers(servers) {}

ServerManager::~ServerManager() {}
