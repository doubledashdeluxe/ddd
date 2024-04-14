#include "SystemRecord.hh"

#include "game/System.hh"

void SystemRecord::applyAspectRatioSetting() {
    System::SetAspectRatio(m_aspectRatio);
}

void SystemRecord::setSecretKart(u16 secretKart) {
    secretKart &= SecretKart::All;

    m_secretKart = secretKart;
}

void SystemRecord::setGameFlag(u16 gameFlag) {
    gameFlag &= GameFlag::All;

    m_gameFlag = gameFlag;
}

void SystemRecord::unlockAll() {
    setSecretKart(SecretKart::All);
    setGameFlag(GameFlag::All);
}

SystemRecord &SystemRecord::Instance() {
    return s_instance;
}
