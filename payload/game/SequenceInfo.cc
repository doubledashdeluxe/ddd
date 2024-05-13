#include "SequenceInfo.hh"

u32 SequenceInfo::getBattleMode() const {
    return m_battleMode;
}

void SequenceInfo::init() {
    REPLACED(init)();

    m_hasOnlineNames = false;
}

SequenceInfo &SequenceInfo::Instance() {
    return s_instance;
}
