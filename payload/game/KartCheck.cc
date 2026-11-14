#include "KartCheck.hh"

#include "game/SequenceInfo.hh"

bool KartCheck::checkBodyHit(u32 i0, u32 i1) const {
    // This returns true if there is no collision
    return SequenceInfo::Instance().isOnlineTA() || REPLACED(checkBodyHit)(i0, i1);
}
