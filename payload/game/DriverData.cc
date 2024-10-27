#include "DriverData.hh"

#include "game/ResMgr.hh"

DriverData *DriverData::Create() {
    ResMgr::SwapForRace(JKRHeap::GetCurrentHeap());

    return REPLACED(Create)();
}
