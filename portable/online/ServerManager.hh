#pragma once

#include "portable/Array.hh"
#include "portable/Optional.hh"
#include "portable/Ring.hh"
#include "portable/crypto/Types.hh"

const u32 MaxServerCount = 32;
const u32 MaxServerNameSize = 32;

typedef Array<char, MaxServerNameSize> ServerName;

class ServerManager {
public:
    class Server {
    public:
        Server(ServerName name, Array<char, 80> address, Optional<u16> port, PublicKey publicKey);
        ~Server();

        const ServerName &name() const;
        const Array<char, 80> &address() const;
        Optional<u16> port() const;
        const PublicKey &publicKey() const;

    private:
        ServerName m_name;
        Array<char, 80> m_address;
        Optional<u16> m_port;
        PublicKey m_publicKey;
    };

    virtual bool isLocked() = 0;
    virtual bool lock() = 0;
    virtual void unlock() = 0;

    u32 serverCount() const;
    const Server &server(u32 index) const;

protected:
    ServerManager(const Ring<Server, MaxServerCount> &servers);
    ~ServerManager();

private:
    const Ring<Server, MaxServerCount> &m_servers;
};
