#include "SysDebug.hh"

SysDebug *SysDebug::GetManager() {
    return s_manager;
}
