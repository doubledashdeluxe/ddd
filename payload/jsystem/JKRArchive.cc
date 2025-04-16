#include "JKRArchive.hh"

#include "jsystem/JKRAramArchive.hh"
#include "jsystem/JKRMemArchive.hh"

#include <common/Bytes.hh>
#include <common/Clock.hh>
#include <common/Log.hh>
#include <common/storage/Storage.hh>
extern "C" {
#include <dolphin/DVD.h>
}
#include <payload/ArchivePatcher.hh>
#include <portable/New.hh>

extern "C" {
#include <assert.h>
#include <stdio.h>
#include <string.h>
}

s32 JKRArchive::getEntrynum() const {
    return m_entrynum;
}

JKRArchive *JKRArchive::Mount(const char *path, u32 mountMode, JKRHeap *heap, u32 mountDirection) {
    return Mount(path, mountMode, heap, mountDirection, true);
}

JKRArchive *JKRArchive::Mount(void *archive, JKRHeap *heap, u32 mountDirection) {
    u32 archiveSize = Archive(reinterpret_cast<u8 *>(archive)).getArchiveSize();
    return Mount(archive, archiveSize, MountMode::Mem, heap, mountDirection, false, true);
}

JKRArchive *JKRArchive::Mount(const char *path, u32 mountMode, JKRHeap *heap, u32 mountDirection,
        bool patchesAllowed) {
    s32 entrynum = DVDConvertPathToEntrynum(path);
    assert(entrynum >= 0);
    return Mount(entrynum, mountMode, heap, mountDirection, patchesAllowed);
}

JKRArchive *JKRArchive::Mount(s32 entrynum, u32 mountMode, JKRHeap *heap, u32 mountDirection,
        bool patchesAllowed) {
    for (JSUPtrLink *link = s_volumeList.getFirstLink(); link; link = link->getNext()) {
        JKRArchive *volume = reinterpret_cast<JKRArchive *>(link->getObjectPtr());
        if (volume->m_entrynum == entrynum) {
            volume->m_mountCount++;
            return volume;
        }
    }

    Array<char, 256> path;
    assert(DVDConvertEntrynumToPath(entrynum, path.values(), path.count()));

    const char *bare = strrchr(path.values(), '/');
    bare = bare ? bare + 1 : path.values();
    INFO("Loading %s...", bare);

    if (!heap) {
        heap = JKRHeap::GetCurrentHeap();
    }
    s32 alignment = mountDirection == MountDirection::Head ? 0x20 : -0x20;
    u8 *archivePtr;
    switch (mountMode) {
    case MountMode::Aram:
        archivePtr = new (heap, alignment) u8[sizeof(JKRAramArchive)];
        break;
    default:
        archivePtr = new (heap, alignment) u8[sizeof(JKRMemArchive)];
        break;
    }

    Array<char, 256> filePath;
    s32 length = snprintf(filePath.values(), filePath.count(), "dvd:%s", path.values());
    assert(length >= 0 && static_cast<size_t>(length) < filePath.count());

    for (;; Clock::WaitMilliseconds(200)) {
        Storage::FileHandle file(filePath.values(), Storage::Mode::Read);
        u64 fileSize;
        if (!file.size(fileSize)) {
            continue;
        }
        if (fileSize > UINT32_MAX) {
            continue;
        }

        UniquePtr<u8[]> archive(new (heap, alignment) u8[fileSize]);
        if (!archive.get()) {
            continue;
        }

        if (!file.read(archive.get(), fileSize, 0)) {
            continue;
        }
        if (!Archive(archive.get()).isValid(fileSize)) {
            continue;
        }

        JKRArchive *volume = Mount(bare, entrynum, archive.get(), fileSize, mountMode, heap,
                mountDirection, true, patchesAllowed, archivePtr);
        if (volume) {
            archive.release();
            return volume;
        }
    }
}

JKRArchive *JKRArchive::Mount(void *archive, u32 archiveSize, u32 mountMode, JKRHeap *heap,
        u32 mountDirection, bool ownsMemory, bool patchesAllowed) {
    return Mount(Archive(reinterpret_cast<u8 *>(archive)), archiveSize, mountMode, heap,
            mountDirection, ownsMemory, patchesAllowed);
}

JKRArchive *JKRArchive::Mount(Archive archive, u32 archiveSize, u32 mountMode, JKRHeap *heap,
        u32 mountDirection, bool ownsMemory, bool patchesAllowed) {
    s32 entrynum = reinterpret_cast<intptr_t>(archive.get());
    for (JSUPtrLink *link = s_volumeList.getFirstLink(); link; link = link->getNext()) {
        JKRArchive *volume = reinterpret_cast<JKRArchive *>(link->getObjectPtr());
        if (volume->m_entrynum == entrynum) {
            volume->m_mountCount++;
            return volume;
        }
    }

    if (ownsMemory) {
        assert(archive.isValid(archiveSize));
    }
    Archive::Tree tree = archive.getTree();
    Archive::Dir dir = tree.getDir(0);
    Array<char, 256> bare;
    snprintf(bare.values(), bare.count(), "%s.arc", dir.getName(tree.getNames()));
    if (ownsMemory) {
        INFO("Loading %s...", bare.values());
    }

    if (!heap) {
        heap = JKRHeap::GetCurrentHeap();
    }
    s32 alignment = mountDirection == MountDirection::Head ? 0x20 : -0x20;
    u8 *archivePtr = new (heap, alignment) u8[sizeof(JKRMemArchive)];

    return Mount(bare.values(), entrynum, archive, archiveSize, mountMode, heap, mountDirection,
            ownsMemory, patchesAllowed, archivePtr);
}

JKRArchive *JKRArchive::Mount(const char *bare, s32 entrynum, Archive archive, u32 archiveSize,
        u32 mountMode, JKRHeap *heap, u32 mountDirection, bool ownsMemory, bool patchesAllowed,
        u8 *archivePtr) {
    s32 alignment = mountDirection == MountDirection::Head ? 0x20 : -0x20;
    if (patchesAllowed) {
        if (!ArchivePatcher::Patch(bare, archive, archiveSize, heap, alignment, ownsMemory)) {
            return nullptr;
        }
    }

    if (ownsMemory) {
        INFO("Loaded %s.", bare);
    }
    switch (mountMode) {
    case MountMode::Aram:
        return new (archivePtr) JKRAramArchive(entrynum, archive, mountDirection);
    default:
        return new (archivePtr) JKRMemArchive(entrynum, archive, mountDirection, ownsMemory);
    }
}
