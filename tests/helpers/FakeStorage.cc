#include "FakeStorage.hh"

#include <cstring>

FakeStorage::FakeStorage() : Storage(nullptr) {
    m_pollCallback = &FakeStorage::add;
    notify();
}

FakeStorage::~FakeStorage() {
    m_pollCallback = &Storage::remove;
    notify();
}

FakeStorage::File::File(FakeStorage *storage, std::string path)
    : m_storage(storage), m_path(path) {}

FakeStorage::File::~File() {
    Storage::File::close();
}

void FakeStorage::File::close() {
    m_storage->m_openFiles.erase(m_path);
}

bool FakeStorage::File::read(void *dst, u32 size, u32 offset) {
    const auto &file = m_storage->m_files[m_path];
    if (offset > file.size() || offset + size < offset || offset + size > file.size()) {
        return false;
    }
    memcpy(dst, file.data() + offset, size);
    return true;
}

bool FakeStorage::File::write(const void *src, u32 size, u32 offset) {
    auto &file = m_storage->m_files[m_path];
    if (offset > file.size()) {
        return false;
    }
    if (offset + size > file.size()) {
        file.resize(offset + size);
    }
    memcpy(file.data() + offset, src, size);
    return true;
}

bool FakeStorage::File::sync() {
    return true;
}

bool FakeStorage::File::truncate(u64 size) {
    auto &file = m_storage->m_files[m_path];
    file.resize(size);
    return true;
}

bool FakeStorage::File::size(u64 &size) {
    const auto &file = m_storage->m_files[m_path];
    size = file.size();
    return true;
}

Storage *FakeStorage::File::storage() {
    return m_storage;
}

FakeStorage::Dir::Dir(FakeStorage *storage, std::string path) : m_storage(storage), m_path(path) {}

FakeStorage::Dir::~Dir() {
    Storage::Dir::close();
}

void FakeStorage::Dir::close() {
    m_storage->m_openDirs.erase(m_path);
}

bool FakeStorage::Dir::read(NodeInfo &nodeInfo) {
    if (m_index >= m_storage->m_files.size() + m_storage->m_dirs.size()) {
        return false;
    }
    const std::string *path;
    if (m_index < m_storage->m_files.size()) {
        path = &std::next(m_storage->m_files.cbegin(), m_index)->first;
        nodeInfo.type = NodeType::File;
    } else {
        path = &*std::next(m_storage->m_dirs.cbegin(), m_index - m_storage->m_files.size());
        nodeInfo.type = NodeType::Dir;
    }
    size_t pos = path->rfind('/');
    if (pos == std::string::npos) {
        return false;
    }
    auto name = path->substr(pos + 1);
    snprintf(nodeInfo.name.values(), nodeInfo.name.count(), "%s", name.c_str());
    m_index++;
    return true;
}

Storage *FakeStorage::Dir::storage() {
    return m_storage;
}

void FakeStorage::poll() {
    (this->*m_pollCallback)();
}

u32 FakeStorage::priority() {
    return 3;
}

const char *FakeStorage::prefix() {
    return "fake:";
}

Storage::File *FakeStorage::openFile(const char *path, u32 mode) {
    if (mode == Mode::Read || mode == Mode::ReadWrite) {
        if (!hasFile(path)) {
            return nullptr;
        }
    } else if (mode == Mode::WriteAlways) {
        if (!hasFile(path)) {
            if (!hasParentDir(path)) {
                return nullptr;
            }
            if (hasDir(path)) {
                return nullptr;
            }
            m_files.emplace(path, 0);
        }
    } else {
        if (hasFile(path)) {
            return nullptr;
        }
        if (!hasParentDir(path)) {
            return nullptr;
        }
        if (hasDir(path)) {
            return nullptr;
        }
        m_files.emplace(path, 0);
    }

    if (m_openFiles.contains(path)) {
        return nullptr;
    }

    auto *file = new File(this, path);
    m_openFiles.emplace(path, file);
    return file;
}

Storage::Dir *FakeStorage::openDir(const char *path) {
    if (!hasDir(path)) {
        return nullptr;
    }

    if (m_openDirs.contains(path)) {
        return nullptr;
    }

    auto *dir = new Dir(this, path);
    m_openDirs.emplace(path, dir);
    return dir;
}

bool FakeStorage::createDir(const char *path, u32 mode) {
    if (hasDir(path)) {
        return mode == Mode::WriteAlways;
    }
    if (!hasParentDir(path)) {
        return false;
    }
    if (hasFile(path)) {
        return false;
    }

    m_dirs.emplace(path);
    return true;
}

bool FakeStorage::rename(const char *srcPath, const char *dstPath) {
    if (hasFile(dstPath) || hasDir(dstPath)) {
        return false;
    }
    if (!hasParentDir(dstPath)) {
        return false;
    }
    if (!hasFile(srcPath)) {
        return false;
    }

    if (m_openFiles.contains(srcPath)) {
        return false;
    }

    auto file = m_files[srcPath];
    m_files.erase(srcPath);
    m_files.emplace(srcPath, file);
    return true;
}

bool FakeStorage::remove(const char *path, u32 mode) {
    if (hasFile(path)) {
        if (m_openFiles.contains(path)) {
            return false;
        }

        m_files.erase(path);
        return true;
    } else if (hasDir(path)) {
        if (m_openDirs.contains(path)) {
            return false;
        }

        std::string prefix = path;
        prefix += '/';
        for (auto [filePath, _] : m_files) {
            if (filePath.starts_with(prefix)) {
                return false;
            }
        }
        for (auto dirPath : m_dirs) {
            if (dirPath.starts_with(prefix)) {
                return false;
            }
        }

        m_dirs.erase(path);
        return true;
    } else {
        return mode == Mode::RemoveAlways;
    }
}

bool FakeStorage::hasFile(const std::string &path) {
    return m_files.contains(path);
}

bool FakeStorage::hasDir(const std::string &path) {
    return path == "" || m_dirs.contains(path);
}

bool FakeStorage::hasParentDir(const std::string &path) {
    size_t pos = path.rfind('/');
    return pos != std::string::npos && hasDir(path.substr(0, pos));
}
