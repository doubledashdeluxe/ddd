#include "CubeReplay.hh"

#include "payload/Lock.hh"

#include <cube/storage/Storage.hh>
extern "C" {
#include <miniz/miniz.h>
}

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

bool CubeReplay::read() {
    Storage::FileHandle file(m_replay.path.values(), Storage::Mode::Read);
    u64 size;
    if (!file.size(size) || size > UINT32_MAX) {
        return m_ok = false;
    }

    tinfl_decompressor decompressor;
    tinfl_init(&decompressor);
    tinfl_status status = TINFL_STATUS_NEEDS_MORE_INPUT;
    u32 compressedOffset = m_replay.offset;
    u32 uncompressedOffset = 0;
    alignas(0x20) Array<u8, 4 * 1024> compressedBuffer;
    alignas(0x20) Array<u8, TINFL_LZ_DICT_SIZE> uncompressedBuffer;
    while (status != TINFL_STATUS_DONE) {
        s32 flags = 0;
        u32 compressedBufferOffset = compressedOffset % compressedBuffer.count();
        u8 *compressed = compressedBuffer.values() + compressedBufferOffset;
        size_t compressedSize = compressedBuffer.count() - compressedBufferOffset;
        if (compressedSize > size - compressedOffset) {
            compressedSize = size - compressedOffset;
        } else {
            flags |= TINFL_FLAG_HAS_MORE_INPUT;
        }
        u32 uncompressedBufferOffset = uncompressedOffset % uncompressedBuffer.count();
        u8 *uncompressed = uncompressedBuffer.values() + uncompressedBufferOffset;
        size_t uncompressedSize = uncompressedBuffer.count() - uncompressedBufferOffset;
        if (status == TINFL_STATUS_NEEDS_MORE_INPUT) {
            if (!file.read(compressed, compressedSize, compressedOffset)) {
                return m_ok = false;
            }
        }
        status = tinfl_decompress(&decompressor, compressed, &compressedSize,
                uncompressedBuffer.values(), uncompressed, &uncompressedSize, flags);
        compressedOffset += compressedSize;
        uncompressedOffset += uncompressedSize;
        switch (status) {
        case TINFL_STATUS_DONE:
            if (compressedOffset != size) {
                return m_ok = false;
            }
            break;
        case TINFL_STATUS_NEEDS_MORE_INPUT:
            if (compressedOffset >= size) {
                return m_ok = false;
            }
            break;
        case TINFL_STATUS_HAS_MORE_OUTPUT:
            if (compressedOffset > size) {
                return m_ok = false;
            }
            break;
        default:
            return m_ok = false;
        }

        while (true) {
            {
                Lock<Mutex> lock(m_mutex);

                for (; uncompressedSize; uncompressed++, uncompressedSize--) {
                    if (!m_buffer.pushBack(*uncompressed)) {
                        break;
                    }
                }
            }

            if (!uncompressedSize) {
                break;
            }

            OSReceiveMessage(&m_queue, nullptr, OS_MESSAGE_BLOCK);

            if (!m_reading) {
                return false;
            }
        }
    }

    return true;
}

void *CubeReplay::Read(void *param) {
    static_cast<CubeReplay *>(param)->read();
    return nullptr;
}
