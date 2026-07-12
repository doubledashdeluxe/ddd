extern "C" {
#include "OSReset.h"
}

#include <cube/ChannelLauncher.hh>
#include <payload/Replace.hh>

extern "C" REPLACE void __OSReboot(u32 /* resetCode */, BOOL /* forceMenu */) {
    ChannelLauncher::Launch();
}
