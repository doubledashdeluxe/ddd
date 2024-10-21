#include "JKRAramArchive.hh"

#include "jsystem/JKRAram.hh"

extern "C" {
#include <assert.h>
#include <string.h>
}

JKRAramArchive::JKRAramArchive(s32 entrynum, Archive archive, u32 mountDirection)
    : JKRArchive(entrynum, MountMode::Aram), m_archive(archive.get()) {
    m_mountDirection = mountDirection;
    m_expandSizes = nullptr;
    m_block.reset(JKRAram::GetHeap()->alloc(archive.getFilesSize(), m_mountDirection - 1));
    assert(JKRAram::MainRamToAram(archive.getFiles(), m_block->getAddress(), archive.getFilesSize(),
            0, 0, nullptr, -1, nullptr));
    Archive::Tree tree(archive.get() + 0x20);
    u32 treeSize = archive.getArchiveSize() - archive.getTreeOffset();
    memmove(tree.get(), archive.getTree().get(), treeSize);
    assert(m_heap->resize(archive.get(), 0x20 + treeSize) >= 0);
    m_tree = tree.get();
    m_dirs = tree.getDir(0).get();
    m_nodes = tree.getNode(0).get();
    m_names = tree.getNames();
    memcpy(&m_signature, "RARC", sizeof(m_signature));
    m_name = Archive::Dir(m_dirs).getName(m_names);
    s_volumeList.prepend(&m_link);
    m_isMounted = true;
}

JKRAramArchive::~JKRAramArchive() {}
