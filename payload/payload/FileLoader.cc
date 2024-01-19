#include "FileLoader.hh"

#include <common/storage/Storage.hh>

void *FileLoader::Load(const char *path, JKRHeap *heap, u32 *size) {
    Storage::FileHandle file(path, Storage::Mode::Read);
    u64 fileSize;
    if (!file.size(fileSize)) {
        return nullptr;
    }
    if (fileSize > UINT32_MAX) {
        return nullptr;
    }

    void *data = heap->alloc(fileSize, 0x20);
    if (!data) {
        return nullptr;
    }

    if (!file.read(data, fileSize, 0x0)) {
        heap->free(data);
        return nullptr;
    }

    if (size) {
        *size = fileSize;
    }
    return data;
}
