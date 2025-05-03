#include "Payload.hh"

#include "payload/ArchiveStorage.hh"
#include "payload/CourseManager.hh"
#include "payload/DVDStorage.hh"
#include "payload/DirCreator.hh"
#include "payload/Lock.hh"
#include "payload/LogFile.hh"
#include "payload/VirtualCard.hh"
#include "payload/WUP028.hh"
#include "payload/crypto/CubeRandom.hh"
#include "payload/network/CubeDNS.hh"
#include "payload/online/ClientK.hh"
#include "payload/online/CubeServerManager.hh"

#include <cube/Clock.hh>
#include <cube/CubeLogger.hh>
#include <cube/Platform.hh>
#include <cube/USB.hh>
#include <cube/VirtualDI.hh>
#include <cube/ios/Resource.hh>
#include <cube/storage/EXISDStorage.hh>
#include <cube/storage/Storage.hh>
#include <cube/storage/USBStorage.hh>
#include <cube/storage/WiiSDStorage.hh>
#include <portable/Log.hh>

void Payload::Run(Context *context) {
    INFO("Started payload.");
    CubeLogger::Init();

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
    CubeServerManager::Init();
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

    if (context->hasVirtualDI) {
        INFO("Initializing virtual disc...");
        VirtualDI::Init();
        while (!VirtualDI::Mount()) {
            Clock::WaitMilliseconds(1000);
        }
        INFO("Initialized virtual disc.");
    }

    INFO("Initializing random number generator...");
    CubeRandom::Init();
    INFO("Initialized random number generator.");

    INFO("Initializing client key...");
    ClientK::Init();
    INFO("Initialized client key.");

    INFO("Initializing DNS...");
    CubeDNS::Init();
    INFO("Initialized DNS.");
}
