#pragma once

#include "payload/Archive.hh"

#include <common/Array.hh>
#include <common/storage/Storage.hh>
#include <jsystem/JKRHeap.hh>

class ArchivePatcher {
public:
    static Archive Patch(const char *bare, Archive archive, u32 archiveSize, JKRHeap *heap,
            s32 alignment, bool &ownsMemory);

private:
    ArchivePatcher(const char *bare, Archive archive, u32 archiveSize, JKRHeap *heap, s32 alignment,
            bool &ownsMemory);
    ~ArchivePatcher();
    Archive patch();
    void addDir(const char *prefix);
    void addDir(u32 rootLength, Array<char, 256> &path, Storage::NodeInfo &nodeInfo);
    void addFile(u32 rootLength, Array<char, 256> &path);
    void addNode(u32 fileSize, const char *name, u32 dirIndex, u32 nodeIndex);
    void grow(u32 size);
    void shrink();

    const char *m_bare;
    Archive m_archive;
    u32 m_archiveSize;
    JKRHeap *m_heap;
    s32 m_alignment;
    bool &m_ownsMemory;
};
