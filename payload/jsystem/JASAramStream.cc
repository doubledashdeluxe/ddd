#include "JASAramStream.hh"

#include "jsystem/JASDriver.hh"
#include "jsystem/JKRAram.hh"

#include <common/Bytes.hh>
extern "C" {
#include <dolphin/DVD.h>
#include <dolphin/OSTime.h>
}
#include <payload/Lock.hh>
extern "C" {
#include <stdio.h>
#include <string.h>
}

JASAramStream::JASAramStream() {}

bool JASAramStream::prepare(s32 entrynum, s32 aramBlockCount) {
    if (!JASDriver::RegisterSubFrameCallback(ChannelProc, this)) {
        return false;
    }

    HeaderLoadTask task;
    task.aramStream = this;
    task.entrynum = entrynum;
    task.aramBufferSize = m_aramBufferSize;
    task.aramBlockCount = aramBlockCount;
    if (!s_loadThread->sendCmdMsg(HeaderLoad, &task, sizeof(task))) {
        JASDriver::RejectCallback(ChannelProc, this);
        return false;
    }

    return true;
}

bool JASAramStream::headerLoad(s32 entrynum, u32 aramBufferSize, s32 aramBlockCount) {
    if (s_fatalErrorFlag) {
        return false;
    }

    if (m_wasCanceled) {
        return false;
    }

    if (!openFile(entrynum)) {
        s_fatalErrorFlag = true;
        return false;
    }

    if (!m_file.read(s_readBuffer, 0x40, 0x0)) {
        s_fatalErrorFlag = true;
        return false;
    }

    m_format = Bytes::ReadBE<u8>(s_readBuffer, 0x09);
    m_channelCount = Bytes::ReadBE<u16>(s_readBuffer, 0x0c);
    m_sampleRate = Bytes::ReadBE<u32>(s_readBuffer, 0x10);
    m_hasLoop = Bytes::ReadBE<u16>(s_readBuffer, 0x0e);
    m_loopStart = Bytes::ReadBE<u32>(s_readBuffer, 0x18);
    m_loopEnd = Bytes::ReadBE<u32>(s_readBuffer, 0x1c);
    m_volume = Bytes::ReadBE<u8>(s_readBuffer, 0x28) / 127.0f;
    m_aramFreeBlockCount = 0;
    m_currentBlock = 0;
    m_aramCurrentBlock = 0;
    m_aramBlockCount = aramBufferSize / s_blockSize / m_channelCount;
    m_aramChannelBlockCount = m_aramBlockCount - 1;
    m_aramLoadBlockCount = m_aramChannelBlockCount;
    if (aramBlockCount < 0 || static_cast<u32>(aramBlockCount) > m_aramLoadBlockCount) {
        aramBlockCount = m_aramLoadBlockCount;
    }

    if (m_wasCanceled) {
        return false;
    }

    LoadTask task;
    task.aramStream = this;
    task._4 = m_aramLoadBlockCount - 1;
    task._8 = aramBlockCount;
    if (!s_loadThread->sendCmdMsg(FirstLoad, &task, sizeof(task))) {
        s_fatalErrorFlag = true;
        return false;
    }

    Lock<NoInterrupts> lock;
    m_aramFreeBlockCount++;

    return true;
}

bool JASAramStream::load() {
    {
        Lock<NoInterrupts> lock;
        m_aramFreeBlockCount--;
    }

    if (s_fatalErrorFlag) {
        return false;
    }

    if (m_wasCanceled) {
        return false;
    }

    u32 samplesPerBlock = m_format == Format::ADPCM ? s_blockSize * 16 / 9 : s_blockSize / 2;
    u32 loopStartBlock = m_loopStart / samplesPerBlock;
    u32 loopEndBlock = (m_loopEnd - 1) / samplesPerBlock;
    if (m_currentBlock > loopEndBlock) {
        return false;
    }

    u32 size = 0x20 + m_channelCount * s_blockSize;
    u32 offset = 0x40 + m_currentBlock * size;
    if (m_currentBlock == loopEndBlock) {
        u64 fileSize;
        if (!m_file.size(fileSize)) {
            s_fatalErrorFlag = true;
            return false;
        }
        size = fileSize - offset;
    }
    if (!m_file.read(s_readBuffer, size, offset)) {
        s_fatalErrorFlag = true;
        return false;
    }

    if (m_wasCanceled) {
        return false;
    }

    for (u32 i = 0; i < m_channelCount; i++) {
        u32 size = Bytes::ReadBE<u32>(s_readBuffer, 0x4);
        u8 *src = s_readBuffer + 0x20 + i * size;
        u32 dst = m_aramBuffer + (i * m_aramBlockCount + m_aramCurrentBlock) * s_blockSize;
        if (!JKRAram::MainRamToAram(src, dst, size, 0, 0, nullptr, -1, nullptr)) {
            s_fatalErrorFlag = true;
            return false;
        }
    }

    m_aramCurrentBlock++;
    if (m_aramCurrentBlock >= m_aramLoadBlockCount) {
        u32 nextEndBlock = m_currentBlock + m_aramLoadBlockCount - 1;
        if (m_hasLoop) {
            while (nextEndBlock > loopEndBlock) {
                nextEndBlock -= loopEndBlock - loopStartBlock;
            }
        }
        if (nextEndBlock == loopEndBlock || nextEndBlock + 2 == loopEndBlock) {
            m_aramLoadBlockCount = m_aramBlockCount;
            OSSendMessage(&m_stateQueue, reinterpret_cast<void *>(5), OS_MESSAGE_BLOCK);
        } else {
            m_aramLoadBlockCount = m_aramBlockCount - 1;
        }

        for (u32 i = 0; i < m_channelCount; i++) {
            m_coefs[0][i] = Bytes::ReadBE<s16>(s_readBuffer, 0x8 + i * 0x4 + 0x0);
            m_coefs[1][i] = Bytes::ReadBE<s16>(s_readBuffer, 0x8 + i * 0x4 + 0x2);
        }

        m_aramCurrentBlock = 0;
    }

    m_currentBlock++;
    if (m_hasLoop && m_currentBlock > loopEndBlock) {
        m_currentBlock = loopStartBlock;
    }

    return true;
}

bool JASAramStream::openFile(s32 entrynum) {
    Array<char, 256> path;
    if (!DVDConvertEntrynumToPath(entrynum, path.values(), path.count())) {
        return false;
    }

    const char *bare = strrchr(path.values(), '/');
    bare = bare ? bare + 1 : path.values();

    Array<char, 256> filePath;
    s32 length = snprintf(filePath.values(), filePath.count(), "main:/ddd/assets/%s", bare);
    if (length >= 0 && static_cast<size_t>(length) < filePath.count()) {
        if (m_file.open(filePath.values(), Storage::Mode::Read)) {
            return true;
        }
    }

    if (!strncmp(bare, "FINALLAP_", strlen("FINALLAP_")) && s_name[0] != '\0') {
        length = snprintf(filePath.values(), filePath.count(), "main:/ddd/assets/%s/%s", bare,
                s_name.values());
        if (length >= 0 && static_cast<size_t>(length) < filePath.count()) {
            if (m_file.open(filePath.values(), Storage::Mode::Read)) {
                return true;
            }
        }
    }

    Array<char, 256> name('\0');
    Array<char, 256> dirPath;
    length = snprintf(dirPath.values(), dirPath.count(), "main:/ddd/assets/%s", bare);
    if (length >= 0 && static_cast<size_t>(length) < dirPath.count()) {
        Storage::DirHandle dir(dirPath.values());
        Storage::NodeInfo nodeInfo;
        for (u32 bestScore = 0; dir.read(nodeInfo);) {
            u32 score = OSGetTime() % 1024;
            if (score >= bestScore) {
                name = nodeInfo.name;
                bestScore = score;
            }
        }
        if (name[0] != '\0') {
            if (!strncmp(bare, "COURSE_", strlen("COURSE_"))) {
                s_name = name;
            }

            length = snprintf(filePath.values(), filePath.count(), "main:/ddd/assets/%s/%s", bare,
                    name.values());
            if (length >= 0 && static_cast<size_t>(length) < filePath.count()) {
                if (m_file.open(filePath.values(), Storage::Mode::Read)) {
                    return true;
                }
            }
        }
    }

    length = snprintf(filePath.values(), filePath.count(), "dvd:%s", path.values());
    if (length >= 0 && static_cast<size_t>(length) < filePath.count()) {
        if (m_file.open(filePath.values(), Storage::Mode::Read)) {
            return true;
        }
    }

    return false;
}

void JASAramStream::HeaderLoad(void *userData) {
    HeaderLoadTask *task = reinterpret_cast<HeaderLoadTask *>(userData);
    task->aramStream->headerLoad(task->entrynum, task->aramBufferSize, task->aramBlockCount);
}

void JASAramStream::Finish(void *userData) {
    JASAramStream *aramStream = reinterpret_cast<JASAramStream *>(userData);
    aramStream->m_file.close();

    REPLACED(Finish)(userData);
}

Array<char, 256> JASAramStream::s_name('\0');
