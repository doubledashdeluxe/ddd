#include "ES.hh"

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

bool ES::sign(const void *data, u32 size, Array<u8, 0x3c> &signature,
        Array<u8, 0x180> &certificate) {
    alignas(0x40) Array<u8, 0x3c> alignedSignature;
    alignas(0x40) Array<u8, 0x180> alignedCertificate;

    alignas(0x20) IoctlvPair pairs[3];
    pairs[0].data = const_cast<void *>(data);
    pairs[0].size = size;
    pairs[1].data = alignedSignature.values();
    pairs[1].size = alignedSignature.count();
    pairs[2].data = alignedCertificate.values();
    pairs[2].size = alignedCertificate.count();

    if (ioctlv(Ioctlv::Sign, 1, 2, pairs) < 0) {
        return false;
    }

    signature = alignedSignature;
    certificate = alignedCertificate;
    return true;
}
