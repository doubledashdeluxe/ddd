#include "MapObjMeteor.hh"

#include "game/SequenceInfo.hh"

void MapObjMeteor::showItem() {
    if (!SequenceInfo::Instance().isOnlineTA()) {
        REPLACED(showItem)();
    }
}
