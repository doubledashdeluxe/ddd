extern "C" {
#include "OS.h"

#include "dolphin/OSError.h"
}

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
        OSReport("Running on Dolphin\n");
    } else {
        OSReport("Running on console\n");
    }

    isInit = true;
}
