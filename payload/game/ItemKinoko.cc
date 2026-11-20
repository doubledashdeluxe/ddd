#include "ItemKinoko.hh"

#include "game/SequenceInfo.hh"

void ItemKinoko::initFall() {
    if (SequenceInfo::Instance().isOnlineTA()) {
        setStateForceDisappear();
        return;
    }

    REPLACED(initFall)();
}
