#include <common/storage/WiiSDStorage.hh>

#include <common/Arena.hh>

WiiSDStorage::WiiSDStorage()
    : IOS::Resource("/dev/sdio/slot0", IOS::Mode::None), SDStorage(nullptr) {
    notify();
}

WiiSDStorage::~WiiSDStorage() {
    if (isContained()) {
        notify();
    }
}

void WiiSDStorage::Init() {
    s_buffer = new (MEM2Arena::Instance(), 0x20) Array<u8, 0x4000>;
}

void WiiSDStorage::poll() {
    if (isContained()) {
        pollRemove();
    } else {
        pollAdd();
    }
}
