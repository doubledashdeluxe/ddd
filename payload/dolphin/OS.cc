extern "C" {
#include "OS.h"
}

#include <common/Console.hh>
#include <common/Log.hh>
#include <common/Platform.hh>
#include <common/VI.hh>
#include <common/ios/Resource.hh>

static bool isInit = false;

extern "C" void OSInit() {
    REPLACED(OSInit)();

    if (isInit) {
        return;
    }

    VI::Init();

    Console::Init();
    INFO("Double Dash Deluxe Payload\n");
    INFO("\n");

    IOS::Resource::Init();
    if (Platform::IsDolphin()) {
        INFO("Running on Dolphin\n");
    } else {
        INFO("Running on console\n");
    }

    Console::Deinit();

    isInit = true;
}
