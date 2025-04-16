#include "ArchiveSwapper.hh"

#include <common/Log.hh>
#include <jsystem/JKRAram.hh>
#include <portable/Align.hh>

extern "C" {
#include <assert.h>
#include <string.h>
}

ArchiveSwapper::ArchiveSwapper(const char *bare, Archive &archive, JKRHeap *heap, u32 minSwapSize,
        u32 maxSwapSize)
    : m_bare(bare), m_archive(archive), m_heap(heap), m_minSwapSize(minSwapSize),
      m_maxSwapSize(maxSwapSize) {}

void ArchiveSwapper::setIsSwappable(const char *path, bool isSwappable) {
    Archive::Tree tree = m_archive.getTree();
    const char *name;
    Archive::Dir dir(nullptr);
    Archive::Node node(nullptr);
    bool exists;
    assert(tree.search(path, name, dir, node, exists));
    assert(exists);
    if (node.isDir()) {
        for (u32 i = 0; i < dir.getNodeCount(); i++) {
            node = dir.getNode(i, tree);
            if (!node.isFile()) {
                continue;
            }
            setIsSwappable(node, isSwappable);
        }
    } else {
        setIsSwappable(node, isSwappable);
    }
}

void ArchiveSwapper::swap(JKRHeap *heap) {
    Archive::Tree tree = m_archive.getTree();
    UniquePtr<u8[]> oldSwap;
    if (m_block.get()) {
        oldSwap.reset(new (heap, 0x20) u8[m_swapSize]);
        assert(JKRAram::AramToMainRam(m_block->getAddress(), oldSwap.get(), m_swapSize, 0, 0,
                nullptr, -1, nullptr));
        for (u32 i = 0; i < tree.getNodeCount(); i++) {
            Archive::Node node = tree.getNode(i);
            if (!node.isFile()) {
                continue;
            }
            if (node.getFileSize() == 0) {
                continue;
            }
            u32 address = node.getFileAddress();
            if (!(address & 1 << 29)) {
                continue;
            }
            address &= ~(1 << 30);
            address &= ~(1 << 29);
            node.setFileOffset(oldSwap.get() + address - m_archive.getFiles());
        }
    }

    UniquePtr<u8[]> newSwap(new (heap, 0x20) u8[m_maxSwapSize]);
    m_swapSize = 0;
    for (u32 i = 0; i < tree.getNodeCount(); i++) {
        Archive::Node node = tree.getNode(i);
        if (!node.isFile()) {
            continue;
        }
        u32 size = AlignUp(node.getFileSize(), 0x20);
        if (size == 0) {
            continue;
        }
        u32 address = node.getFileAddress();
        node.setFileAddress(0);
        if (!(address & 1 << 30)) {
            continue;
        }
        if (m_swapSize + size > m_maxSwapSize) {
            continue;
        }
        memcpy(newSwap.get() + m_swapSize, node.getFile(m_archive.getFiles()), size);
        node.setFileAddress(1 << 29 | m_swapSize);
        m_swapSize += size;
    }

    m_block.reset();
    m_block.reset(JKRAram::GetHeap()->alloc(m_swapSize, JKRAramHeap::AllocMode::Head));
    assert(JKRAram::MainRamToAram(newSwap.get(), m_block->getAddress(), m_swapSize, 0, 0, nullptr,
            -1, nullptr));

    u8 *dst = m_archive.get() + 0x20;
    while (true) {
        u32 nodeIndex;
        u8 *src = nullptr;
        for (u32 i = 0; i < tree.getNodeCount(); i++) {
            Archive::Node node = tree.getNode(i);
            if (!node.isFile()) {
                continue;
            }
            if (node.getFileSize() == 0) {
                continue;
            }
            if (node.getFileAddress() != 0) {
                continue;
            }
            u32 offset = node.getFileOffset();
            if (offset >= dst - m_archive.getFiles() || offset >= m_archive.getFilesSize()) {
                if (!src || offset < src - m_archive.getFiles()) {
                    nodeIndex = i;
                    src = m_archive.getFiles() + offset;
                }
            }
        }
        if (!src) {
            break;
        }
        Archive::Node node = tree.getNode(nodeIndex);
        u32 size = AlignUp(node.getFileSize(), 0x20);
        assert(dst + size - m_archive.getFiles() <= m_archive.getFilesSize());
        memmove(dst, src, size);
        node.setFileOffset(dst - m_archive.getFiles());
        dst += size;
    }

    if (!oldSwap.get()) {
        m_archive.setFilesSize(m_archive.getFilesSize() - m_minSwapSize);
        assert(dst - m_archive.getFiles() <= m_archive.getFilesSize());
        tree = Archive::Tree(m_archive.getFiles() + m_archive.getFilesSize());
        u32 treeSize = m_archive.getArchiveSize() - m_archive.getTreeOffset();
        memmove(tree.get(), m_archive.getTree().get(), treeSize);
        m_archive.setTree(tree);
        u32 archiveSize = tree.get() + treeSize - m_archive.get();
        if (archiveSize != m_archive.getArchiveSize()) {
            if (m_heap->resize(m_archive.get(), archiveSize) >= 0) {
                DEBUG("Resized %s from 0x%lx to 0x%lx", m_bare, m_archive.getArchiveSize(),
                        archiveSize);
                m_archive.setArchiveSize(archiveSize);
            }
        }
    }
}

void ArchiveSwapper::setIsSwappable(Archive::Node node, bool isSwappable) {
    u32 address = node.getFileAddress();
    address &= ~(1 << 30);
    address |= isSwappable << 30;
    node.setFileAddress(address);
}
