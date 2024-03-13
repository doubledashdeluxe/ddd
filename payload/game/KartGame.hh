#pragma once

#include "game/KartBody.hh"

#include <payload/Replace.hh>

class KartGame {
public:
    KartGame();

    void REPLACED(doStatus)();
    REPLACE void doStatus();
    void REPLACED(doChange)();
    REPLACE void doChange();

private:
    KartBody *m_body;
    u8 _04[0x44 - 0x04];
};
size_assert(KartGame, 0x44);
