#include <cube/storage/USBStorage.hh>

#include <cube/Arena.hh>

void USBStorage::Init() {
    s_instance = new (MEM2Arena::Instance(), 0x20) USBStorage;
}
