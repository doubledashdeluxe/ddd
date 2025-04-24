#pragma once

#include "payload/Archive.hh"

#include <cube/storage/Storage.hh>
#include <jsystem/JKRHeap.hh>
#include <portable/Array.hh>

class ArchivePatcher {
public:
    static bool Patch(const char *bare, Archive &archive, u32 archiveSize, JKRHeap *heap,
            s32 alignment, bool &ownsMemory);

private:
    ArchivePatcher(const char *bare, Archive &archive, u32 archiveSize, JKRHeap *heap,
            s32 alignment, bool &ownsMemory);
    ~ArchivePatcher();
    bool patch();
    bool addDir(const char *prefix);
    bool addDir(u32 rootLength, Array<char, 256> &path, Storage::NodeInfo &nodeInfo);
    bool addFile(u32 rootLength, Array<char, 256> &path);
    bool addNode(u32 fileSize, const char *name, u32 dirIndex, u32 nodeIndex);
    bool grow(u32 size);
    void shrink();

    const char *m_bare;
    Archive &m_archive;
    u32 m_archiveSize;
    JKRHeap *m_heap;
    s32 m_alignment;
    bool &m_ownsMemory;
};
