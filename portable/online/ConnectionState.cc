#include "ConnectionState.hh"

ConnectionState::ConnectionState(const ClientPlatform &platform, Array<u8, 32> serverPK)
    : m_platform(platform), m_serverPK(serverPK) {}

ConnectionState::~ConnectionState() {}
