#include "Connection.hh"

#include "portable/online/ConnectionStateDNS.hh"

Connection::Connection(const ClientPlatform &platform, Array<u8, 32> serverPK,
        const Array<char, 32> &name, u16 port)
    : m_state(new(platform.allocator) ConnectionStateDNS(platform, serverPK, name, port)) {}

Connection::~Connection() {}

Optional<Address> Connection::address() const {
    return m_state->address();
}

void Connection::reset() {
    while (updateState(m_state->reset())) {}
}

bool Connection::read(ServerStateReader &reader, u8 *buffer, u32 size, const Address &address) {
    bool ok;
    while (updateState(m_state->read(reader, buffer, size, address, ok)) && !ok) {}
    return ok;
}

bool Connection::write(ClientStateWriter &writer, u8 *buffer, u32 &size, Address &address) {
    bool ok;
    while (updateState(m_state->write(writer, buffer, size, address, ok)) && !ok) {}
    return ok;
}

bool Connection::updateState(ConnectionState &nextState) {
    if (&nextState == m_state.get()) {
        return false;
    }

    m_state.reset(&nextState);
    return true;
}
