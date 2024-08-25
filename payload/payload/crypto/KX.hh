#pragma once

#include "payload/crypto/Session.hh"
#include "payload/crypto/SymmetricState.hh"

#include <common/Array.hh>

class KX {
public:
    enum {
        M1Size = 96,
        M2Size = 48,
    };

    class ClientState {
    public:
        ClientState();
        ~ClientState();

    private:
        SymmetricState m_symmetricState;
        Array<u8, 32> m_k;
        Array<u8, 32> m_ephemeralK;

        friend class KX;
    };

    static void IK1(const Array<u8, 32> &clientK, const Array<u8, 32> &serverPK, u8 m1[M1Size],
            ClientState &clientState);
    static bool IK2(const Array<u8, 32> &serverK, const u8 m1[M1Size], u8 m2[M2Size],
            Array<u8, 32> &clientPK, Session &serverSession);
    static bool IK3(ClientState &clientState, const u8 m2[M2Size], Session &clientSession);

private:
    KX();
};
