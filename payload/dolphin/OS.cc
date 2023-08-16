extern "C" {
#include "OS.h"
}

#include <common/Log.hh>
#include <common/Platform.hh>
#include <common/ios/Resource.hh>

static bool isInit = false;

extern "C" void OSInit() {
    REPLACED(OSInit)();

    if (isInit) {
        return;
    }

    IOS::Resource::Init();
    if (Platform::IsDolphin()) {
        INFO("Running on Dolphin\n");
    } else {
        INFO("Running on console\n");
    }

    isInit = true;
}
