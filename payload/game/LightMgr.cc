#include "LightMgr.hh"

LightMgr *LightMgr::Instance() {
    return s_instance;
}
