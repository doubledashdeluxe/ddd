#pragma once

#include <jsystem/JKRHeap.hh>

class FileLoader {
public:
    static void *Load(const char *path, JKRHeap *heap);

private:
    FileLoader();
};
