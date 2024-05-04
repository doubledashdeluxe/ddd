#include "Payload.hh"

#include "payload/ArchiveStorage.hh"
#include "payload/AssetsDirCreator.hh"
#include "payload/CourseManager.hh"
#include "payload/DVDStorage.hh"
#include "payload/Lock.hh"
#include "payload/LogFile.hh"
#include "payload/VirtualCard.hh"
#include "payload/WUP028.hh"

#include <common/Clock.hh>
#include <common/Console.hh>
#include <common/Log.hh>
#include <common/Platform.hh>
#include <common/USB.hh>
#include <common/VirtualDI.hh>
#include <common/ios/Resource.hh>
#include <common/storage/SDStorage.hh>
#include <common/storage/Storage.hh>
#include <common/storage/USBStorage.hh>

void Payload::Run(Context *context) {
    INFO("Started payload.");

    INFO("Initializing IOS...");
    IOS::Resource::Init();
    INFO("Initialized IOS.");

    INFO("Initializing clock...");
    Clock::Init();
    INFO("Initialized clock.");

    INFO("Initializing WUP-028...");
    WUP028::Init();
    INFO("Initialized WUP-028.");

    INFO("Initializing assets directory creator...");
    AssetsDirCreator::Init();
    INFO("Initialized assets directory creator.");

    INFO("Initializing course manager...");
    CourseManager::Init();
    INFO("Initialized course manager.");

    INFO("Initializing virtual memory cards...");
    VirtualCard::Init();
    INFO("Initialized virtual memory cards.");

    INFO("Initializing storage...");
    Storage::Init();
    INFO("Initialized storage.");

    INFO("Initializing DVD storage...");
    DVDStorage::Init();
    INFO("Initialized DVD storage.");

    INFO("Initializing common archive storage...");
    ArchiveStorage::Init("carc:", context->commonArchive, context->commonArchiveSize);
    INFO("Initialized common archive storage.");

    INFO("Initializing localized archive storage...");
    ArchiveStorage::Init("larc:", context->localizedArchive, context->localizedArchiveSize);
    INFO("Initialized localized archive storage.");

    INFO("Initializing USB storage...");
    USBStorage::Init();
    INFO("Initialized USB storage.");

    INFO("Initializing USB...");
    USB::Init();
    INFO("Initialized USB.");

    INFO("Initializing SD storage...");
    SDStorage::Init();
    INFO("Initialized SD storage.");

    INFO("Initializing log file...");
    LogFile::Init();
    INFO("Initialized log file.");

    if (context->hasVirtualDI) {
        INFO("Initializing virtual disc...");
        VirtualDI::Init();
        while (!VirtualDI::Mount()) {
            Clock::WaitMilliseconds(1000);
        }
        INFO("Initialized virtual disc.");
    }

    Console::Instance()->setIsDirect(false);
}
