#include "VirtualCard.hh"

#include <cube/Arena.hh>
#include <cube/Clock.hh>
#include <cube/DiscID.hh>
extern "C" {
#include <dolphin/OSTime.h>
}
#include <portable/Bytes.hh>
#include <portable/CardPath.hh>
#include <portable/String.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
}

s32 VirtualCard::freeBlocks(s32 *bytesNotUsed, s32 *filesNotUsed) {
    if (!m_isMounted) {
        return CARD_RESULT_NOCARD;
    }

    *bytesNotUsed = 16 * 1024 * 1024;
    *filesNotUsed = m_files.count();
    for (u32 fileNo = 0; fileNo < m_files.count(); fileNo++) {
        if (m_files[fileNo].isValid) {
            (*filesNotUsed)--;
        }
    }
    return CARD_RESULT_READY;
}

s32 VirtualCard::check() {
    if (!m_isMounted) {
        return CARD_RESULT_NOCARD;
    }

    return CARD_RESULT_READY;
}

s32 VirtualCard::probeEx(s32 *memSize, s32 *sectorSize) {
    *memSize = 16 * 8;
    *sectorSize = SectorSize;
    return CARD_RESULT_READY;
}

s32 VirtualCard::mount(void * /* workArea */, CARDCallback /* detachCallback */) {
    for (u32 fileNo = 0; fileNo < m_files.count(); fileNo++) {
        m_files[fileNo].isValid = false;
    }

    Storage::CreateDir(m_dirPath.values(), Storage::Mode::WriteAlways);

    u32 validFileCount = 0;
    Storage::NodeInfo nodeInfo;
    for (Storage::DirHandle dir(m_dirPath.values());
            validFileCount < m_files.count() && dir.read(nodeInfo);) {
        if (nodeInfo.type != Storage::NodeType::File) {
            continue;
        }

        if (!String::EndsWith(nodeInfo.name.values(), ".gci")) {
            continue;
        }

        Array<char, 256> path;
        snprintf(path.values(), path.count(), "%s/%s", m_dirPath.values(), nodeInfo.name.values());
        Storage::FileHandle file(path.values(), Storage::Mode::Read);
        CARDStat stat;
        if (!ReadStat(file, stat)) {
            continue;
        }

        m_files[validFileCount].stat = stat;
        m_files[validFileCount].path = path;
        m_files[validFileCount].isValid = true;
        validFileCount++;
    }

    m_isMounted = true;
    return CARD_RESULT_READY;
}

s32 VirtualCard::unmount() {
    m_isMounted = false;
    return CARD_RESULT_READY;
}

s32 VirtualCard::format() {
    // Purposely not implemented
    return CARD_RESULT_READY;
}

s32 VirtualCard::fastOpen(s32 fileNo, CARDFileInfo *fileInfo) {
    if (!m_isMounted) {
        return CARD_RESULT_NOCARD;
    }

    if (!m_files[fileNo].isValid) {
        return CARD_RESULT_NOFILE;
    }

    fileInfo->chan = m_chan;
    fileInfo->fileNo = fileNo;
    return CARD_RESULT_READY;
}

s32 VirtualCard::open(const char *fileName, CARDFileInfo *fileInfo) {
    if (strlen(fileName) >= CARD_FILENAME_MAX) {
        return CARD_RESULT_NAMETOOLONG;
    }

    if (!m_isMounted) {
        return CARD_RESULT_NOCARD;
    }

    for (u32 fileNo = 0; fileNo < m_files.count(); fileNo++) {
        if (!m_files[fileNo].isValid) {
            continue;
        }

        if (strcmp(m_files[fileNo].stat.fileName, fileName)) {
            continue;
        }

        fileInfo->chan = m_chan;
        fileInfo->fileNo = fileNo;
        return CARD_RESULT_READY;
    }

    return CARD_RESULT_NOFILE;
}

s32 VirtualCard::close(CARDFileInfo * /* fileInfo */) {
    return CARD_RESULT_READY;
}

s32 VirtualCard::create(const char *fileName, u32 size, CARDFileInfo *fileInfo) {
    if (strlen(fileName) >= CARD_FILENAME_MAX) {
        return CARD_RESULT_NAMETOOLONG;
    }

    if (!m_isMounted) {
        return CARD_RESULT_NOCARD;
    }

    for (u32 fileNo = 0; fileNo < m_files.count(); fileNo++) {
        if (!m_files[fileNo].isValid) {
            continue;
        }

        if (!strcmp(m_files[fileNo].stat.fileName, fileName)) {
            return CARD_RESULT_EXIST;
        }
    }

    for (u32 fileNo = 0; fileNo < m_files.count(); fileNo++) {
        if (m_files[fileNo].isValid) {
            continue;
        }

        CARDStat stat;
        snprintf(stat.fileName, sizeof(stat.fileName), "%s", fileName);
        stat.length = size;
        stat.time = Clock::TicksToSeconds(OSGetTime());
        memcpy(stat.gameName, DiscID::Get().gameID, sizeof(stat.gameName));
        memcpy(stat.company, DiscID::Get().makerID, sizeof(stat.company));
        stat.bannerFormat = 0;
        stat.iconAddr = UINT32_MAX;
        stat.iconFormat = 0;
        stat.iconSpeed = 0;
        stat.commentAddr = UINT32_MAX;

        Array<char, 256> path = getPath(stat);
        Storage::FileHandle file(path.values(), Storage::Mode::WriteNew);
        if (!WriteStat(file, stat)) {
            return CARD_RESULT_NOCARD;
        }
        if (!file.truncate(FileHeaderSize + size)) {
            return CARD_RESULT_NOCARD;
        }

        m_files[fileNo].stat = stat;
        m_files[fileNo].path = path;
        m_files[fileNo].isValid = true;

        fileInfo->chan = m_chan;
        fileInfo->fileNo = fileNo;
        return CARD_RESULT_READY;
    }

    return CARD_RESULT_INSSPACE;
}

s32 VirtualCard::read(CARDFileInfo *fileInfo, void *addr, s32 length, s32 offset) {
    if (!m_isMounted) {
        return CARD_RESULT_NOCARD;
    }

    if (!m_files[fileInfo->fileNo].isValid) {
        return CARD_RESULT_NOFILE;
    }

    Storage::FileHandle file(m_files[fileInfo->fileNo].path.values(), Storage::Mode::Read);
    if (!file.read(addr, length, FileHeaderSize + offset)) {
        return CARD_RESULT_NOCARD;
    }

    return CARD_RESULT_READY;
}

s32 VirtualCard::write(CARDFileInfo *fileInfo, const void *addr, s32 length, s32 offset) {
    if (!m_isMounted) {
        return CARD_RESULT_NOCARD;
    }

    if (!m_files[fileInfo->fileNo].isValid) {
        return CARD_RESULT_NOFILE;
    }

    Storage::FileHandle file(m_files[fileInfo->fileNo].path.values(), Storage::Mode::ReadWrite);
    if (!file.write(addr, length, FileHeaderSize + offset)) {
        return CARD_RESULT_NOCARD;
    }

    return CARD_RESULT_READY;
}

s32 VirtualCard::fastRemove(s32 fileNo) {
    if (!m_isMounted) {
        return CARD_RESULT_NOCARD;
    }

    if (!m_files[fileNo].isValid) {
        return CARD_RESULT_NOFILE;
    }

    if (!Storage::Remove(m_files[fileNo].path.values(), Storage::Mode::RemoveExisting)) {
        return CARD_RESULT_NOCARD;
    }

    m_files[fileNo].isValid = false;
    return CARD_RESULT_READY;
}

s32 VirtualCard::remove(const char *fileName) {
    if (strlen(fileName) >= CARD_FILENAME_MAX) {
        return CARD_RESULT_NAMETOOLONG;
    }

    if (!m_isMounted) {
        return CARD_RESULT_NOCARD;
    }

    for (u32 fileNo = 0; fileNo < m_files.count(); fileNo++) {
        if (!m_files[fileNo].isValid) {
            continue;
        }

        if (strcmp(m_files[fileNo].stat.fileName, fileName)) {
            continue;
        }

        if (!Storage::Remove(m_files[fileNo].path.values(), Storage::Mode::RemoveExisting)) {
            return CARD_RESULT_NOCARD;
        }

        m_files[fileNo].isValid = false;
        return CARD_RESULT_READY;
    }

    return CARD_RESULT_NOFILE;
}

s32 VirtualCard::getStatus(s32 fileNo, CARDStat *stat) {
    if (!m_isMounted) {
        return CARD_RESULT_NOCARD;
    }

    if (!m_files[fileNo].isValid) {
        return CARD_RESULT_NOFILE;
    }

    *stat = m_files[fileNo].stat;
    return CARD_RESULT_READY;
}

s32 VirtualCard::setStatus(s32 fileNo, CARDStat *stat) {
    if (!m_isMounted) {
        return CARD_RESULT_NOCARD;
    }

    if (!m_files[fileNo].isValid) {
        return CARD_RESULT_NOFILE;
    }

    stat->time = Clock::TicksToSeconds(OSGetTime());
    Storage::FileHandle file(m_files[fileNo].path.values(), Storage::Mode::ReadWrite);
    if (!WriteStat(file, *stat)) {
        return CARD_RESULT_NOCARD;
    }

    m_files[fileNo].stat = *stat;
    return CARD_RESULT_READY;
}

s32 VirtualCard::rename(const char *oldName, const char *newName) {
    if (!m_isMounted) {
        return CARD_RESULT_NOCARD;
    }

    for (u32 fileNo = 0; fileNo < m_files.count(); fileNo++) {
        if (!m_files[fileNo].isValid) {
            continue;
        }

        if (!strcmp(m_files[fileNo].stat.fileName, newName)) {
            return CARD_RESULT_EXIST;
        }
    }

    for (u32 fileNo = 0; fileNo < m_files.count(); fileNo++) {
        if (!m_files[fileNo].isValid) {
            continue;
        }

        if (strcmp(m_files[fileNo].stat.fileName, oldName)) {
            continue;
        }

        CARDStat &stat = m_files[fileNo].stat;
        snprintf(stat.fileName, sizeof(stat.fileName), "%s", newName);
        Array<char, 256> newPath = getPath(stat);
        if (!Storage::Rename(m_files[fileNo].path.values(), newPath.values())) {
            snprintf(stat.fileName, sizeof(stat.fileName), "%s", oldName);
            return CARD_RESULT_NOCARD;
        }
        m_files[fileNo].path = newPath;

        stat.time = Clock::TicksToSeconds(OSGetTime());
        Storage::FileHandle file(newPath.values(), Storage::Mode::ReadWrite);
        if (!WriteStat(file, stat)) {
            return CARD_RESULT_NOCARD;
        }

        return CARD_RESULT_READY;
    }

    return CARD_RESULT_NOFILE;
}

void VirtualCard::Init() {
    for (u32 chan = 0; chan < s_instances.count(); chan++) {
        s_instances[chan].reset(new (MEM1Arena::Instance(), 0x4) VirtualCard(chan));
    }
}

VirtualCard *VirtualCard::Instance(s32 chan) {
    return s_instances[chan].get();
}

VirtualCard::VirtualCard(s32 chan) : m_chan(chan), m_isMounted(false) {
    snprintf(m_dirPath.values(), m_dirPath.count(), "main:/ddd/card %c",
            static_cast<char>('a' + m_chan));
}

void VirtualCard::onAdd(const char * /* prefix */) {}

void VirtualCard::onRemove(const char *prefix) {
    if (strcmp(prefix, "main:")) {
        return;
    }

    m_isMounted = false;
}

Array<char, 256> VirtualCard::getPath(CARDStat &stat) {
    return CardPath::Get(m_dirPath.values(), stat.fileName, stat.gameName, stat.company);
}

bool VirtualCard::ReadStat(Storage::FileHandle &file, CARDStat &stat) {
    Array<u8, FileHeaderSize> header;

    if (!file.read(header.values(), header.count(), 0x0)) {
        return false;
    }

    memcpy(stat.fileName, header.values() + 0x08, sizeof(stat.fileName));
    stat.length = Bytes::ReadBE<u16>(header.values(), 0x38) * SectorSize;
    stat.time = Bytes::ReadBE<u32>(header.values(), 0x28);
    memcpy(stat.gameName, header.values() + 0x00, sizeof(stat.gameName));
    memcpy(stat.company, header.values() + 0x04, sizeof(stat.company));
    stat.bannerFormat = Bytes::ReadBE<u8>(header.values(), 0x07);
    stat.iconAddr = Bytes::ReadBE<u32>(header.values(), 0x2c);
    stat.iconFormat = Bytes::ReadBE<u16>(header.values(), 0x30);
    stat.iconSpeed = Bytes::ReadBE<u16>(header.values(), 0x32);
    stat.commentAddr = Bytes::ReadBE<u32>(header.values(), 0x3c);

    CARDUpdateIconOffsets(header.values(), &stat);

    return true;
}

bool VirtualCard::WriteStat(Storage::FileHandle &file, CARDStat &stat) {
    Array<u8, FileHeaderSize> header(0x00);

    memcpy(header.values() + 0x08, stat.fileName, sizeof(stat.fileName));
    Bytes::WriteBE<u16>(header.values(), 0x38, stat.length / SectorSize);
    Bytes::WriteBE<u32>(header.values(), 0x28, stat.time);
    memcpy(header.values() + 0x00, stat.gameName, sizeof(stat.gameName));
    memcpy(header.values() + 0x04, stat.company, sizeof(stat.company));
    Bytes::WriteBE<u8>(header.values(), 0x07, stat.bannerFormat);
    Bytes::WriteBE<u32>(header.values(), 0x2c, stat.iconAddr);
    Bytes::WriteBE<u16>(header.values(), 0x30, stat.iconFormat);
    Bytes::WriteBE<u16>(header.values(), 0x32, stat.iconSpeed);
    Bytes::WriteBE<u32>(header.values(), 0x3c, stat.commentAddr);

    CARDUpdateIconOffsets(header.values(), &stat);

    if (!file.write(header.values(), header.count(), 0x0)) {
        return false;
    }

    return true;
}

Array<UniquePtr<VirtualCard>, CARD_NUM_CHANS> VirtualCard::s_instances;
