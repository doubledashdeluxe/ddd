#include <common/storage/USBStorage.hh>

#include <common/Arena.hh>

void USBStorage::Init() {
    s_instance = new (MEM2Arena::Instance(), 0x20) USBStorage;
}
