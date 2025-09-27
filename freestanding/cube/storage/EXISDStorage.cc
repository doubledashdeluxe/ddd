#include <cube/storage/EXISDStorage.hh>

EXISDStorage::EXISDStorage(u32 index)
    : SDStorage(nullptr), m_index(index), m_channel(IndexToChannel(index)),
      m_device(IndexToDevice(index)), m_wasDetached(false), m_queue(nullptr) {
    notify();
}

EXISDStorage::~EXISDStorage() {
    if (isContained()) {
        notify();
    }
}

void EXISDStorage::poll() {
    if (isContained()) {
        pollRemove();
    } else {
        pollAdd();
    }
}

bool EXISDStorage::dispatch(struct Transfer *transfer) {
    return execute(transfer);
}
