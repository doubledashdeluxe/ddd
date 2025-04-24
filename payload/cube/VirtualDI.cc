#include <cube/VirtualDI.hh>

#include <cube/DiscID.hh>

extern "C" {
#include <string.h>
}

bool VirtualDI::ReadDiscID(Storage::FileHandle &file) {
    alignas(0x20) DiscID discID;
    if (!file.read(&discID, sizeof(discID), 0x0)) {
        return false;
    }
    if (memcmp(&discID, &DiscID::Get(), sizeof(DiscID))) {
        return false;
    }
    return true;
}
