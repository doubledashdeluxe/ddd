#include "JKRArchive.hh"

#include "jsystem/JKRMemArchive.hh"

extern "C" {
#include <dolphin/DVD.h>
}

JKRArchive *JKRArchive::Mount(const char *path, u32 /* mountMode */, JKRHeap *heap,
        u32 mountDirection) {
    return Mount(path, heap, mountDirection, true);
}

JKRArchive *JKRArchive::Mount(void *archive, JKRHeap *heap, u32 mountDirection) {
    return Mount(archive, heap, mountDirection, true);
}

JKRArchive *JKRArchive::Mount(const char *path, JKRHeap *heap, u32 mountDirection,
        bool patchesAllowed) {
    s32 entrynum = DVDConvertPathToEntrynum(path);
    if (entrynum < 0) {
        return nullptr;
    } else {
        return Mount(entrynum, heap, mountDirection, patchesAllowed);
    }
}

JKRArchive *JKRArchive::Mount(s32 entrynum, JKRHeap *heap, u32 mountDirection,
        bool patchesAllowed) {
    if (!heap) {
        heap = JKRHeap::GetCurrentHeap();
    }
    for (JSUPtrLink *link = s_volumeList.getFirstLink(); link; link = link->getNext()) {
        JKRArchive *archive = reinterpret_cast<JKRArchive *>(link->getObjectPtr());
        if (archive->m_entrynum == entrynum && archive->m_heap == heap) {
            archive->m_mountCount++;
            return archive;
        }
    }
    s32 alignment = mountDirection == MountDirection::Head ? 4 : -4;
    return new (heap, alignment) JKRMemArchive(entrynum, mountDirection, patchesAllowed);
}

JKRArchive *JKRArchive::Mount(void *archive, JKRHeap *heap, u32 mountDirection,
        bool patchesAllowed) {
    if (!heap) {
        heap = JKRHeap::GetCurrentHeap();
    }
    s32 entrynum = reinterpret_cast<intptr_t>(archive);
    for (JSUPtrLink *link = s_volumeList.getFirstLink(); link; link = link->getNext()) {
        JKRArchive *archive = reinterpret_cast<JKRArchive *>(link->getObjectPtr());
        if (archive->m_entrynum == entrynum && archive->m_heap == heap) {
            archive->m_mountCount++;
            return archive;
        }
    }
    s32 alignment = mountDirection == MountDirection::Head ? 4 : -4;
    return new (heap, alignment) JKRMemArchive(archive, mountDirection, patchesAllowed);
}
