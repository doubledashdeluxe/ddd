#include "Payload.hh"

#include "payload/ArchiveStorage.hh"
#include "payload/CourseManager.hh"
#include "payload/DVDStorage.hh"
#include "payload/DirCreator.hh"
#include "payload/Lock.hh"
#include "payload/LogFile.hh"
#include "payload/VirtualCard.hh"
#include "payload/WUP028.hh"
#include "payload/crypto/Random.hh"
#include "payload/network/DNS.hh"
#include "payload/online/ClientK.hh"
#include "payload/online/ServerManager.hh"

#include <common/Clock.hh>
#include <common/Log.hh>
#include <common/Platform.hh>
#include <common/USB.hh>
#include <common/VirtualDI.hh>
#include <common/ios/Resource.hh>
#include <common/storage/EXISDStorage.hh>
#include <common/storage/Storage.hh>
#include <common/storage/USBStorage.hh>
#include <common/storage/WiiSDStorage.hh>

void Payload::Run(Context *context) {
    INFO("Started payload.");

    if (!Platform::IsGameCube()) {
        INFO("Initializing IOS...");
        IOS::Resource::Init();
        INFO("Initialized IOS.");
    }

    INFO("Initializing clock...");
    Clock::Init();
    INFO("Initialized clock.");

    if (!Platform::IsGameCube()) {
        INFO("Initializing WUP-028...");
        WUP028::Init();
        INFO("Initialized WUP-028.");
    }

    INFO("Initializing directory creator...");
    DirCreator::Init();
    INFO("Initialized directory creator.");

    INFO("Initializing course manager...");
    CourseManager::Init();
    INFO("Initialized course manager.");

    INFO("Initializing server manager...");
    ServerManager::Init();
    INFO("Initialized server manager.");

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

    INFO("Initializing EXI SD storage...");
    EXISDStorage::Init();
    INFO("Initialized EXI SD storage.");

    if (!Platform::IsGameCube()) {
        INFO("Initializing USB storage...");
        USBStorage::Init();
        INFO("Initialized USB storage.");

        INFO("Initializing USB...");
        USB::Init();
        INFO("Initialized USB.");

        INFO("Initializing Wii SD storage...");
        WiiSDStorage::Init();
        INFO("Initialized Wii SD storage.");
    }

    INFO("Initializing log file...");
    LogFile::Init();
    INFO("Initialized log file.");

    INFO("Initializing random number generator...");
    Random::Init();
    INFO("Initialized random number generator.");

    INFO("Initializing client key...");
    ClientK::Init();
    INFO("Initialized client key.");

    if (context->hasVirtualDI) {
        INFO("Initializing virtual disc...");
        VirtualDI::Init();
        while (!VirtualDI::Mount()) {
            Clock::WaitMilliseconds(1000);
        }
        INFO("Initialized virtual disc.");
    }

    INFO("Initializing DNS...");
    DNS::Init();
    INFO("Initialized DNS.");
}
