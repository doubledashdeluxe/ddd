#include "ServerConnectionState.hh"

ServerConnectionState::ServerConnectionState(const ServerPlatform &platform)
    : m_platform(platform) {}

ServerConnectionState::~ServerConnectionState() = default;
