#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

class KartGame {
public:
    KartGame();

    void REPLACED(doStatus)();
    REPLACE void doStatus();
    void REPLACED(doChange)();
    REPLACE void doChange();

private:
    class KartBody *m_body;
    u8 _04[0x0b - 0x04];

public:
    u8 m_changeTimer;

private:
    u8 _0c[0x44 - 0x0c];
};
size_assert(KartGame, 0x44);
