#include <common/storage/USBStorage.hh>

#include <common/Arena.hh>
#include <payload/Mutex.hh>

void USBStorage::Init() {
    s_mutex = new (MEM2Arena::Instance(), 0x4) Mutex;
    s_instance = new (MEM2Arena::Instance(), 0x20) USBStorage;
}
