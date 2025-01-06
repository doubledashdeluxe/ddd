#include <common/storage/WiiSDStorage.hh>

#include <common/Arena.hh>

WiiSDStorage::WiiSDStorage()
    : IOS::Resource("/dev/sdio/slot0", IOS::Mode::None), FATStorage(s_mutex) {
    m_pollCallback = &WiiSDStorage::pollAdd;
    notify();
}

WiiSDStorage::~WiiSDStorage() {
    if (isContained()) {
        m_pollCallback = &WiiSDStorage::pollRemove;
        notify();
    }
}

void WiiSDStorage::Init() {
    s_buffer = new (MEM2Arena::Instance(), 0x20) Array<u8, 0x4000>;
}

void WiiSDStorage::poll() {
    (this->*m_pollCallback)();
}
