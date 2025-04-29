#pragma once

#include <jsystem/JKRAramBlock.hh>
#include <jsystem/JKRHeap.hh>
#include <portable/Archive.hh>
#include <portable/UniquePtr.hh>

class ArchiveSwapper {
public:
    ArchiveSwapper(const char *bare, Archive &archive, JKRHeap *heap, u32 minSwapSize,
            u32 maxSwapSize);
    ~ArchiveSwapper();
    void setIsSwappable(const char *path, bool isSwappable);
    void swap(JKRHeap *heap);

private:
    void setIsSwappable(Archive::Node node, bool isSwappable);

    const char *m_bare;
    Archive &m_archive;
    JKRHeap *m_heap;
    u32 m_minSwapSize;
    u32 m_maxSwapSize;
    UniquePtr<JKRAramBlock> m_block;
    u32 m_swapSize;
};
