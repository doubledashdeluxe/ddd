#include "ES.hh"

#include "common/Array.hh"
#include "common/Bytes.hh"

ES::ES() : IOS::Resource("/dev/es", IOS::Mode::None) {}

ES::~ES() {}

bool ES::ok() const {
    return Resource::ok();
}

bool ES::launchTitle(u64 titleID, const Array<u8, 0xd8> &ticketView) {
    alignas(0x20) Array<u8, 0xd8> alignedTicketView = ticketView;

    alignas(0x20) IoctlvPair pairs[2];
    pairs[0].data = &titleID;
    pairs[0].size = sizeof(titleID);
    pairs[1].data = alignedTicketView.values();
    pairs[1].size = alignedTicketView.count();

    return ioctlvReboot(Ioctlv::LaunchTitle, 2, pairs);
}
