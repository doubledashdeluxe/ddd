#include "J3DVertexBuffer.hh"

#include "jsystem/J3DSys.hh"

#include <cube/Memory.hh>

void J3DVertexBuffer::setArray() {
    J3DSys &sys = J3DSys::Instance();
    sys._10c = Memory::CachedToPhysical(_2c);
    sys._110 = Memory::CachedToPhysical(_30);
    sys._114 = Memory::CachedToPhysical(_34);
}
