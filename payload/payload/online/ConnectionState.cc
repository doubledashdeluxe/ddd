#include "ConnectionState.hh"

ConnectionState::ConnectionState(JKRHeap *heap, Array<u8, 32> serverPK)
    : m_heap(heap), m_serverPK(serverPK) {}

ConnectionState::~ConnectionState() {}
