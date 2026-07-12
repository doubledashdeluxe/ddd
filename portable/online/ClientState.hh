#pragma once

#include "portable/online/ClientPlatform.hh"
#include "portable/online/ClientReadHandler.hh"
#include "portable/online/ClientStateModeWriteInfo.hh"
#include "portable/online/ClientStatePackWriteInfo.hh"
#include "portable/online/ClientStatePollWriteInfo.hh"
#include "portable/online/ClientStateRaceWriteInfo.hh"
#include "portable/online/ClientStateRoomWriteInfo.hh"
#include "portable/online/ClientStateServerWriteInfo.hh"
#include "portable/online/ClientStateTeamWriteInfo.hh"
#include "portable/online/ClientStateUpdateWriteInfo.hh"
#include "portable/online/Connection.hh"

class ClientState {
public:
    template <typename D>
    class Reader : public ConnectionState::Reader {
    private:
        bool isValid(const u8 *buffer, u32 size, u32 &offset) override {
            ServerStateReader<D> *reader = static_cast<D *>(this);
            return reader->isValid(buffer, size, offset);
        }

        void read(const u8 *buffer, u32 &offset) override {
            ServerStateReader<D> *reader = static_cast<D *>(this);
            reader->read(buffer, offset);
        }
    };

public:
    template <typename D>
    class Writer : public ConnectionState::Writer {
    private:
        bool write(u8 *buffer, u32 size, u32 &offset) override {
            ClientStateWriter<D> *writer = static_cast<D *>(this);
            return writer->write(buffer, size, offset);
        }
    };

    ClientState(const ClientPlatform &platform);
    virtual ~ClientState();
    virtual bool needsSockets() = 0;
    virtual ClientState &read(ClientReadHandler &handler) = 0;
    virtual ClientState &writeStateIdle();
    virtual ClientState &writeStateServer(const ClientStateServerWriteInfo &writeInfo);
    virtual ClientState &writeStateUpdate(const ClientStateUpdateWriteInfo &writeInfo);
    virtual ClientState &writeStateMode(const ClientStateModeWriteInfo &writeInfo);
    virtual ClientState &writeStatePack(const ClientStatePackWriteInfo &writeInfo);
    virtual ClientState &writeStateRoom(const ClientStateRoomWriteInfo &writeInfo);
    virtual ClientState &writeStateTeam(const ClientStateTeamWriteInfo &writeInfo);
    virtual ClientState &writeStatePoll(const ClientStatePollWriteInfo &writeInfo);
    virtual ClientState &writeStateRace(const ClientStateRaceWriteInfo &writeInfo);
    virtual ClientState &writeStateError();

protected:
    void read(ConnectionState::Reader &reader);
    void write(ConnectionState::Writer &writer);

private:
    void checkSocket();

protected:
    const ClientPlatform &m_platform;
    Ring<UniquePtr<Connection>, MaxServerCount> m_connections;
    u32 m_readIndex;
    u32 m_writeIndex;
};
