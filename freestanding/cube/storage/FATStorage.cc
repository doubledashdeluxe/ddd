#include <cube/storage/FATStorage.hh>

#include <cube/Panic.hh>

FATStorage::~FATStorage() {}

extern "C" DWORD get_fattime() {
    PANIC("Unexpected call to get_fattime");
}
