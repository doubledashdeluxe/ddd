extern "C" {
#include "CARD.h"

#include "dolphin/DSP.h"
}

#include <cube/DCache.hh>
#include <cube/Memory.hh>
#include <cube/Platform.hh>
#include <payload/VirtualCard.hh>
#include <portable/Bytes.hh>

struct CARDControl {
    u8 _000[0x030 - 0x000];
    DSPTaskInfo taskInfo;
    u8 *workArea;
    u8 _084[0x110 - 0x084];
};
size_assert(CARDControl, 0x110);

static Array<VirtualCard *, CARD_NUM_CHANS> s_virtualCards(nullptr);
static Array<VirtualCard *, CARD_NUM_CHANS> s_nextVirtualCards(nullptr);

extern "C" REPLACE void CARDInitCallback(DSPTaskInfo *taskInfo) {
    CARDControl *card = container_of(taskInfo, CARDControl, taskInfo);
    u8 *workArea = card->workArea;
    if (!Platform::IsGameCube()) {
        u32 inputAddr = Bytes::ReadBE<u32>(workArea, 0x00);
        u32 aramAddr = Bytes::ReadBE<u32>(workArea, 0x08);
        u32 outputAddr = Bytes::ReadBE<u32>(workArea, 0x0c);
        inputAddr = Memory::CachedToPhysical(reinterpret_cast<void *>(inputAddr)) | 0x80000000;
        aramAddr |= 0x10000000;
        outputAddr = Memory::CachedToPhysical(reinterpret_cast<void *>(outputAddr)) | 0x80000000;
        Bytes::WriteBE<u32>(workArea, 0x00, inputAddr);
        Bytes::WriteBE<u32>(workArea, 0x08, aramAddr);
        Bytes::WriteBE<u32>(workArea, 0x0c, outputAddr);
        DCache::Flush(workArea, 0x10);
    }

    DSPSendMailToDSP(0xff000000);
    while (DSPCheckMailToDSP()) {}
    DSPSendMailToDSP(Memory::CachedToPhysical(workArea));
    while (DSPCheckMailToDSP()) {}
}

extern "C" s32 CARDFreeBlocks(s32 chan, s32 *bytesNotUsed, s32 *filesNotUsed) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->freeBlocks(bytesNotUsed, filesNotUsed);
    }

    return REPLACED(CARDFreeBlocks)(chan, bytesNotUsed, filesNotUsed);
}

extern "C" s32 CARDCheck(s32 chan) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->check();
    }

    return REPLACED(CARDCheck)(chan);
}

extern "C" s32 CARDProbeEx(s32 chan, s32 *memSize, s32 *sectorSize) {
    if (REPLACED(CARDProbeEx)(chan, memSize, sectorSize) == CARD_RESULT_READY) {
        s_nextVirtualCards[chan] = nullptr;
        return CARD_RESULT_READY;
    }

    s_nextVirtualCards[chan] = VirtualCard::Instance(chan);
    return s_nextVirtualCards[chan]->probeEx(memSize, sectorSize);
}

extern "C" s32 CARDMount(s32 chan, void *workArea, CARDCallback detachCallback) {
    s_virtualCards[chan] = s_nextVirtualCards[chan];

    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->mount(workArea, detachCallback);
    }

    return REPLACED(CARDMount)(chan, workArea, detachCallback);
}

extern "C" s32 CARDUnmount(s32 chan) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->unmount();
    }

    return REPLACED(CARDUnmount)(chan);
}

extern "C" s32 CARDFormat(s32 chan) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->format();
    }

    return REPLACED(CARDFormat)(chan);
}

extern "C" s32 CARDFastOpen(s32 chan, s32 fileNo, CARDFileInfo *fileInfo) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->fastOpen(fileNo, fileInfo);
    }

    return REPLACED(CARDFastOpen)(chan, fileNo, fileInfo);
}

extern "C" s32 CARDOpen(s32 chan, const char *fileName, CARDFileInfo *fileInfo) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->open(fileName, fileInfo);
    }

    return REPLACED(CARDOpen)(chan, fileName, fileInfo);
}

extern "C" s32 CARDClose(CARDFileInfo *fileInfo) {
    if (s_virtualCards[fileInfo->chan]) {
        return s_virtualCards[fileInfo->chan]->close(fileInfo);
    }

    return REPLACED(CARDClose)(fileInfo);
}

extern "C" s32 CARDCreate(s32 chan, const char *fileName, u32 size, CARDFileInfo *fileInfo) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->create(fileName, size, fileInfo);
    }

    return REPLACED(CARDCreate)(chan, fileName, size, fileInfo);
}

extern "C" s32 CARDRead(CARDFileInfo *fileInfo, void *addr, s32 length, s32 offset) {
    if (s_virtualCards[fileInfo->chan]) {
        return s_virtualCards[fileInfo->chan]->read(fileInfo, addr, length, offset);
    }

    return REPLACED(CARDRead)(fileInfo, addr, length, offset);
}

extern "C" s32 CARDWrite(CARDFileInfo *fileInfo, const void *addr, s32 length, s32 offset) {
    if (s_virtualCards[fileInfo->chan]) {
        return s_virtualCards[fileInfo->chan]->write(fileInfo, addr, length, offset);
    }

    return REPLACED(CARDWrite)(fileInfo, addr, length, offset);
}

extern "C" s32 CARDFastDelete(s32 chan, s32 fileNo) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->fastRemove(fileNo);
    }

    return REPLACED(CARDFastDelete)(chan, fileNo);
}

extern "C" s32 CARDDelete(s32 chan, const char *fileName) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->remove(fileName);
    }

    return REPLACED(CARDDelete)(chan, fileName);
}

extern "C" s32 CARDGetStatus(s32 chan, s32 fileNo, CARDStat *stat) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->getStatus(fileNo, stat);
    }

    return REPLACED(CARDGetStatus)(chan, fileNo, stat);
}

extern "C" s32 CARDSetStatus(s32 chan, s32 fileNo, CARDStat *stat) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->setStatus(fileNo, stat);
    }

    return REPLACED(CARDSetStatus)(chan, fileNo, stat);
}

extern "C" s32 CARDRename(s32 chan, const char *oldName, const char *newName) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->rename(oldName, newName);
    }

    return REPLACED(CARDRename)(chan, oldName, newName);
}
