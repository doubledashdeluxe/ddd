#include <cube/storage/EXISDStorage.hh>

EXISDStorage::EXISDStorage(u32 channel) : SDStorage(nullptr), m_channel(channel), m_queue(nullptr) {
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
