#pragma once

#include <jsystem/JKRHeap.hh>

class FileLoader {
public:
    static void *Load(const char *path, JKRHeap *heap, u32 *size = nullptr);

private:
    FileLoader();
};
