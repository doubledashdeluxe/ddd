#include <common/storage/FATStorage.hh>

#include <common/Panic.hh>

FATStorage::~FATStorage() {}

extern "C" DWORD get_fattime() {
    PANIC("Unexpected call to get_fattime");
}
