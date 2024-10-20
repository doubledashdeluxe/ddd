#include "JKRMemArchive.hh"

extern "C" {
#include <string.h>
}

JKRMemArchive::JKRMemArchive(s32 entrynum, Archive archive, u32 mountDirection, bool ownsMemory)
    : JKRArchive(entrynum, MountMode::Mem), m_archive(archive.get()), m_files(archive.getFiles()),
      m_ownsMemory(ownsMemory) {
    m_mountDirection = mountDirection;
    Archive::Tree tree = archive.getTree();
    m_tree = tree.get();
    m_dirs = tree.getDir(0).get();
    m_nodes = tree.getNode(0).get();
    m_names = tree.getNames();
    memcpy(&m_signature, "RARC", sizeof(m_signature));
    m_name = Archive::Dir(m_dirs).getName(m_names);
    s_volumeList.prepend(&m_link);
    m_isMounted = true;
}
