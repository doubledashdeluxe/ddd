#include "J3DModelData.hh"

#include "jsystem/J3DSys.hh"

#include <cube/Memory.hh>

void J3DModelData::syncJ3DSysPointers() {
    J3DSys &sys = J3DSys::Instance();
    sys._058 = _6c;
    sys._10c = Memory::CachedToPhysical(_a0);
    sys._110 = Memory::CachedToPhysical(_a4);
    sys._114 = Memory::CachedToPhysical(_ac);
}
