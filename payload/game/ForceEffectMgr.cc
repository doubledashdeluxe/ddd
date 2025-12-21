#include "ForceEffectMgr.hh"

void ForceEffectMgr::Destroy() {
    s_instance.reset();
}
