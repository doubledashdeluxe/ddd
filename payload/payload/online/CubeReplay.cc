#include "CubeReplay.hh"

#include "payload/Lock.hh"

#include <cube/storage/Storage.hh>

CubeReplay::CubeReplay(const ReplayManager::Replay &replay, u32 clientIndex)
    : Replay(replay, clientIndex)
    , m_reading(true) {
    OSInitMessageQueue(&m_queue, m_messages.values(), m_messages.count());
    OSCreateThread(&m_thread, Read, this, m_stack.values() + m_stack.count(), m_stack.count(), 19,
            0);
    OSResumeThread(&m_thread);
}

CubeReplay::~CubeReplay() {
    m_reading = false;
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
    OSJoinThread(&m_thread, nullptr);
}

bool CubeReplay::read(u8 *buffer, u32 &size) {
    Lock<Mutex> lock(m_mutex);

    if (m_buffer.count() < size && !OSIsThreadTerminated(&m_thread)) {
        OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
        return false;
    }

    size = Min(m_buffer.count(), size);
    for (u32 i = 0; i < size; i++) {
        buffer[i] = m_buffer[i];
    }
    return true;
}

void CubeReplay::seek(u32 offset) {
    Lock<Mutex> lock(m_mutex);

    for (u32 i = 0; i < offset; i++) {
        m_buffer.popFront();
    }
}

void CubeReplay::read() {
    Storage::FileHandle file(m_replay.path.values(), Storage::Mode::Read);
    u64 size;
    if (!file.size(size) || size > UINT32_MAX) {
        m_ok = false;
        return;
    }

    for (u32 offset = m_replay.offset; offset < size;) {
        alignas(0x20) u8 chunk[4 * 1024];
        u32 chunkSize = Min<u32>(size - offset, sizeof(chunk));
        if (!file.read(chunk, chunkSize, offset)) {
            m_ok = false;
            return;
        }

        while (true) {
            OSReceiveMessage(&m_queue, nullptr, OS_MESSAGE_BLOCK);

            if (!m_reading) {
                return;
            }

            Lock<Mutex> lock(m_mutex);

            if (m_buffer.count() + chunkSize < m_buffer.Capacity) {
                for (u32 i = 0; i < chunkSize; i++) {
                    m_buffer.pushBack(chunk[i]);
                }
                break;
            }
        }

        offset += chunkSize;
    }
}

void *CubeReplay::Read(void *param) {
    static_cast<CubeReplay *>(param)->read();
    return nullptr;
}
