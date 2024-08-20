#include <common/storage/SDStorage.hh>

#include <common/Arena.hh>

SDStorage::SDStorage() : IOS::Resource("/dev/sdio/slot0", IOS::Mode::None), FATStorage(s_mutex) {
    m_pollCallback = &SDStorage::pollAdd;
    notify();
}

SDStorage::~SDStorage() {
    if (isContained()) {
        m_pollCallback = &SDStorage::pollRemove;
        notify();
    }
}

void SDStorage::Init() {
    s_buffer = new (MEM2Arena::Instance(), 0x20) Array<u8, 0x4000>;
}

void SDStorage::poll() {
    (this->*m_pollCallback)();
}
