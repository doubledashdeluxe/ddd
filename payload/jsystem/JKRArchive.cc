#include "JKRArchive.hh"

#include "jsystem/JKRMemArchive.hh"

#include <common/Bytes.hh>
#include <common/Log.hh>
#include <common/storage/Storage.hh>
extern "C" {
#include <dolphin/DVD.h>
}
#include <payload/ArchivePatcher.hh>

extern "C" {
#include <assert.h>
#include <stdio.h>
#include <string.h>
}

JKRArchive *JKRArchive::Mount(const char *path, u32 /* mountMode */, JKRHeap *heap,
        u32 mountDirection) {
    return Mount(path, heap, mountDirection, true);
}

JKRArchive *JKRArchive::Mount(void *archive, JKRHeap *heap, u32 mountDirection) {
    u32 archiveSize = Bytes::ReadBE<u32>(reinterpret_cast<u8 *>(archive), 0x04);
    return Mount(archive, archiveSize, heap, mountDirection, true);
}

JKRArchive *JKRArchive::Mount(const char *path, JKRHeap *heap, u32 mountDirection,
        bool patchesAllowed) {
    s32 entrynum = DVDConvertPathToEntrynum(path);
    assert(entrynum >= 0);
    return Mount(entrynum, heap, mountDirection, patchesAllowed);
}

JKRArchive *JKRArchive::Mount(s32 entrynum, JKRHeap *heap, u32 mountDirection,
        bool patchesAllowed) {
    Array<char, 256> path;
    assert(DVDConvertEntrynumToPath(entrynum, path.values(), path.count()));

    const char *bare = strrchr(path.values(), '/');
    bare = bare ? bare + 1 : path.values();
    INFO("Loading %s...", bare);

    Array<char, 256> filePath;
    s32 length = snprintf(filePath.values(), filePath.count(), "dvd:%s", path.values());
    assert(length >= 0 && static_cast<size_t>(length) < filePath.count());

    Storage::FileHandle file(filePath.values(), Storage::Mode::Read);
    u64 fileSize;
    assert(file.size(fileSize) && fileSize <= UINT32_MAX);

    if (!heap) {
        heap = JKRHeap::GetCurrentHeap();
    }
    s32 alignment = mountDirection == MountDirection::Head ? 0x20 : -0x20;
    Archive archive(new (heap, alignment) u8[fileSize]);
    assert(archive.get());

    assert(file.read(archive.get(), fileSize, 0));
    assert(archive.isValid(fileSize));

    return Mount(bare, entrynum, archive, fileSize, heap, mountDirection, true, patchesAllowed);
}

JKRArchive *JKRArchive::Mount(void *archive, u32 archiveSize, JKRHeap *heap, u32 mountDirection,
        bool patchesAllowed) {
    return Mount(Archive(reinterpret_cast<u8 *>(archive)), archiveSize, heap, mountDirection,
            patchesAllowed);
}

JKRArchive *JKRArchive::Mount(Archive archive, u32 archiveSize, JKRHeap *heap, u32 mountDirection,
        bool patchesAllowed) {
    assert(archive.isValid(archiveSize));
    Archive::Tree tree = archive.getTree();
    Archive::Dir dir = tree.getDir(0);
    Array<char, 256> bare;
    snprintf(bare.values(), bare.count(), "%s.arc", dir.getName(tree.getNames()));
    INFO("Loading %s...", bare.values());

    s32 entrynum = reinterpret_cast<intptr_t>(archive.get());
    if (!heap) {
        heap = JKRHeap::GetCurrentHeap();
    }

    return Mount(bare.values(), entrynum, archive, archiveSize, heap, mountDirection, false,
            patchesAllowed);
}

JKRArchive *JKRArchive::Mount(const char *bare, s32 entrynum, Archive archive, u32 archiveSize,
        JKRHeap *heap, u32 mountDirection, bool ownsMemory, bool patchesAllowed) {
    for (JSUPtrLink *link = s_volumeList.getFirstLink(); link; link = link->getNext()) {
        JKRArchive *volume = reinterpret_cast<JKRArchive *>(link->getObjectPtr());
        if (volume->m_entrynum == entrynum) {
            volume->m_mountCount++;
            return volume;
        }
    }

    s32 alignment = mountDirection == MountDirection::Head ? 0x20 : -0x20;
    if (patchesAllowed) {
        archive = ArchivePatcher::Patch(bare, archive, archiveSize, heap, alignment, ownsMemory);
    }

    INFO("Loaded %s.", bare);
    return new (heap, alignment) JKRMemArchive(entrynum, archive, mountDirection, ownsMemory);
}
