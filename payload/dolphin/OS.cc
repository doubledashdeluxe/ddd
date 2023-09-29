extern "C" {
#include "OS.h"
}

#include <common/Console.hh>
#include <common/Log.hh>
#include <common/Platform.hh>
#include <common/USB.hh>
#include <common/VI.hh>
#include <common/ios/Resource.hh>
#include <common/storage/Storage.hh>
#include <common/storage/USBStorage.hh>
#include <payload/WUP028.hh>

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

    INFO("Initializing IOS...");
    IOS::Resource::Init();
    INFO(" done.\n");

    INFO("Initializing WUP-028...");
    WUP028::Init();
    INFO(" done.\n");

    INFO("Initializing storage...");
    Storage::Init();
    INFO(" done.\n");

    INFO("Initializing USB storage...");
    USBStorage::Init();
    INFO(" done.\n");

    INFO("Initializing USB...");
    USB::Init();
    INFO(" done.\n");

    Console::Deinit();

    isInit = true;
}
