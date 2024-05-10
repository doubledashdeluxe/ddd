#include "AppMgr.hh"

s32 AppMgr::CurrentApp() {
    return s_currentApp;
}

void AppMgr::Request(u32 requests) {
    s_requests |= requests;
}
