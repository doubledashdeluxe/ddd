#pragma once

#include "portable/Array.hh"
#include "portable/Ring.hh"

const u32 MaxServerCount = 32;
const u32 MaxServerNameSize = 32;

class ServerManager {
public:
    class Server {
    public:
        Server(Array<char, MaxServerNameSize> name, Array<char, 32> address, u16 port,
                Array<u8, 32> publicKey);
        ~Server();

        const Array<char, MaxServerNameSize> &name() const;
        const Array<char, 32> &address() const;
        u16 port() const;
        const Array<u8, 32> &publicKey() const;

    private:
        Array<char, MaxServerNameSize> m_name;
        Array<char, 32> m_address;
        u16 m_port;
        Array<u8, 32> m_publicKey;
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
