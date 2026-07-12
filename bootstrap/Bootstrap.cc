#include "Bootstrap.hh"

#include <cube/ChannelInstaller.hh>
#include <cube/ChannelLauncher.hh>
#include <cube/Console.hh>
#include <cube/CubeLogger.hh>
#include <cube/ES.hh>
#include <cube/Platform.hh>
#include <cube/VI.hh>
#include <cube/ios/KernelUID.hh>
#include <formats/Version.hh>
#include <portable/Log.hh>

extern "C" const u8 channel[];
extern "C" const size_t channel_size;

extern "C" u32 iosVersion;

void Bootstrap::Run() {
    VI::Init();
    Console::Init(VI::Instance());
    CubeLogger::Init();
    INFO("Double Dash Deluxe Bootstrap [%s]", Version);

    if (iosVersion >> 16 != 58 && iosVersion >> 16 != 59) {
        ERROR("Double Dash Deluxe needs IOS58 (or IOS59).");
        ERROR("Please perform a Wii System Update or use the IOS58 Installer to install");
        ERROR("IOS58.");
        return;
    }

    Platform::s_iosVersion = iosVersion;

    {
        INFO("Setting UID to kernel...");
        IOS::KernelUID kernelUID(false);
        if (!kernelUID.ok()) {
            ERROR("Failed to set UID to kernel.");
            return;
        }
        INFO("Set UID to kernel.");

        if (!ChannelInstaller::Install(channel, channel_size)) {
            return;
        }

        INFO("Setting UID back to PPC...");
    }
    INFO("Set UID back to PPC.");

    ChannelLauncher::Launch();
    ERROR("Failed to launch the channel.");
}
