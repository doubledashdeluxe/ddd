#include "ConnectionState.hh"

ConnectionState::ConnectionState(Allocator &allocator, Array<u8, 32> serverPK)
    : m_allocator(allocator), m_serverPK(serverPK) {}

ConnectionState::~ConnectionState() {}
