#include "ConnectionState.hh"

ConnectionState::ConnectionState(const ClientPlatform &platform, PublicKey serverPK)
    : m_platform(platform), m_serverPK(serverPK) {}

ConnectionState::~ConnectionState() {}
