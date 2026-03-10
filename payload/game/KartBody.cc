#include "KartBody.hh"

KartGame *KartBody::getGame() const {
    return m_game;
}

u32 KartBody::getGameStatus() const {
    return m_gameStatus;
}

u8 KartBody::getDriver() const {
    return m_driver;
}

u8 KartBody::getIndex() const {
    return m_index;
}
