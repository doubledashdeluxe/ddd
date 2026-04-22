#include "ServerManager.hh"

ServerManager::Server::Server(ServerName name, Array<char, 32> address, u16 port,
        PublicKey publicKey)
    : m_name(name)
    , m_address(address)
    , m_port(port)
    , m_publicKey(publicKey) {}

ServerManager::Server::~Server() {}

const ServerName &ServerManager::Server::name() const {
    return m_name;
}

const Array<char, 32> &ServerManager::Server::address() const {
    return m_address;
}

u16 ServerManager::Server::port() const {
    return m_port;
}

const PublicKey &ServerManager::Server::publicKey() const {
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
