#include <common/VirtualDI.hh>

#include <common/DiscID.hh>

bool VirtualDI::ReadDiscID(Storage::FileHandle &file) {
    if (!file.read(&DiscID::Get(), sizeof(DiscID), 0x0)) {
        return false;
    }
    if (!DiscID::IsValid()) {
        return false;
    }
    return true;
}
