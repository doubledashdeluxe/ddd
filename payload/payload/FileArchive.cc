#include "FileArchive.hh"

FileArchive::Dir::Dir(FileArchive &archive, const char *path, bool &ok)
    : Archive::Dir(nullptr), m_archive(archive) {
    ok = false;
    const char *name;
    Archive::Node node(nullptr);
    bool exists;
    if (!m_archive.m_tree.search(path, name, *this, node, exists)) {
        return;
    }
    if (!exists) {
        return;
    }
    ok = true;
}

char *FileArchive::Dir::getName() const {
    return Archive::Dir::getName(m_archive.m_tree.getNames());
}

FileArchive::Node FileArchive::Dir::getNode(u32 index) const {
    return Node(Archive::Dir::getNode(index, m_archive.m_tree), m_archive);
}

FileArchive::Dir::Dir(Archive::Dir dir, FileArchive &archive)
    : Archive::Dir(dir), m_archive(archive) {}

FileArchive::Node::Node(FileArchive &archive, const char *path, bool &ok)
    : Archive::Node(nullptr), m_archive(archive) {
    ok = false;
    const char *name;
    Archive::Dir dir(nullptr);
    bool exists;
    if (!m_archive.m_tree.search(path, name, dir, *this, exists)) {
        return;
    }
    if (!exists) {
        return;
    }
    ok = true;
}

char *FileArchive::Node::Node::getName() const {
    return Archive::Node::getName(m_archive.m_tree.getNames());
}

FileArchive::Dir FileArchive::Node::getDir() const {
    return Dir(Archive::Node::getDir(m_archive.m_tree), m_archive);
}

bool FileArchive::Node::getFile(void *file) const {
    u32 offset = m_archive.m_archive.getFilesOffset() + getFileOffset();
    return m_archive.m_file.read(file, getFileSize(), offset);
}

void *FileArchive::Node::getFile(JKRHeap *heap, s32 alignment) const {
    heap = heap ? heap : m_archive.m_heap;
    void *file = heap->alloc(getFileSize(), alignment);
    if (!file) {
        return nullptr;
    }
    if (!getFile(file)) {
        heap->free(file);
        return nullptr;
    }
    return file;
}

FileArchive::Node::Node(Archive::Node node, FileArchive &archive)
    : Archive::Node(node), m_archive(archive) {}

FileArchive::FileArchive(JKRHeap *heap, s32 alignment, const char *path, bool &ok)
    : m_heap(heap), m_file(path, Storage::Mode::Read), m_archive(nullptr), m_tree(nullptr) {
    ok = false;

    u64 fileSize;
    if (!m_file.size(fileSize)) {
        return;
    }
    if (fileSize > UINT32_MAX) {
        return;
    }

    m_archive = Archive(reinterpret_cast<u8 *>(heap->alloc(0x20, alignment)));
    if (!m_archive.get()) {
        return;
    }
    if (!m_file.read(m_archive.get(), 0x20, 0x0)) {
        return;
    }
    if (!m_archive.isHeaderValid(fileSize)) {
        return;
    }

    m_tree = Archive::Tree(reinterpret_cast<u8 *>(heap->alloc(m_archive.getTreeSize(), alignment)));
    if (!m_tree.get()) {
        return;
    }
    if (!m_file.read(m_tree.get(), m_archive.getTreeSize(), m_archive.getTreeOffset())) {
        return;
    }
    if (!m_tree.isValid(m_archive.getTreeSize(), m_archive.getFilesSize())) {
        return;
    }

    ok = true;
}

FileArchive::~FileArchive() {
    if (m_tree.get()) {
        m_heap->free(m_tree.get());
    }
    if (m_archive.get()) {
        m_heap->free(m_archive.get());
    }
}
