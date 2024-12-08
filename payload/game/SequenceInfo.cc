#include "SequenceInfo.hh"

u32 SequenceInfo::getBattleMode() const {
    return m_battleMode;
}

SequenceInfo &SequenceInfo::Instance() {
    return s_instance;
}
