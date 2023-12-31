#include <common/storage/FATStorage.hh>

#include <payload/DOSTime.hh>

Array<FATStorage *, FF_VOLUMES> FATStorage::s_volumes(nullptr); // TODO move to common

extern "C" DWORD get_fattime() {
    return DOSTime::Now();
}
