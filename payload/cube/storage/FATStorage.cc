#include <cube/storage/FATStorage.hh>

#include <payload/DOSTime.hh>

extern "C" DWORD get_fattime() {
    return DOSTime::Now();
}
