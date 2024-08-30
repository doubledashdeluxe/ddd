#include "Connection.hh"

#include "payload/online/ConnectionStateDNS.hh"

Connection::Connection(JKRHeap *heap, Array<u8, 32> serverPK, const char *name)
    : m_heap(heap), m_state(new(m_heap, 0x4) ConnectionStateDNS(m_heap, serverPK, name)) {}

Connection::~Connection() {}

void Connection::reset() {
    while (updateState(m_state->reset())) {}
}

bool Connection::read(u8 *buffer, u32 size, const Socket::Address &address) {
    bool ok;
    while (updateState(m_state->read(buffer, size, address, ok)) && !ok) {}
    return ok;
}

bool Connection::write(u8 *buffer, u32 &size, Socket::Address &address) {
    bool ok;
    while (updateState(m_state->write(buffer, size, address, ok)) && !ok) {}
    return ok;
}

bool Connection::updateState(ConnectionState &nextState) {
    if (&nextState == m_state.get()) {
        return false;
    }

    m_state.reset(&nextState);
    return true;
}
