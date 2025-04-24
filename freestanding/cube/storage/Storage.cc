#include <cube/storage/Storage.hh>

void Storage::Init() {}

void Storage::notify() {
    poll();
}

void Storage::remove() {
    removeWithoutLocking();
}

void Storage::add() {
    addWithoutLocking();
}

Storage::StorageHandle::StorageHandle(const char *path) : m_storage(nullptr), m_prefix(nullptr) {
    acquireWithoutLocking(path);
}

Storage::StorageHandle::StorageHandle(const FileHandle &file)
    : m_storage(nullptr), m_prefix(nullptr) {
    acquireWithoutLocking(file);
}

Storage::StorageHandle::StorageHandle(const DirHandle &dir)
    : m_storage(nullptr), m_prefix(nullptr) {
    acquireWithoutLocking(dir);
}

Storage::StorageHandle::~StorageHandle() {}
