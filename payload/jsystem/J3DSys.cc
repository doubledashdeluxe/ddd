#include "J3DSys.hh"

void J3DSys::setViewMtx(const Mtx34 viewMtx) {
    PSMTXCopy(viewMtx, m_viewMtx);
}

J3DSys &J3DSys::Instance() {
    return s_instance;
}
