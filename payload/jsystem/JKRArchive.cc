#include "JKRArchive.hh"

JKRArchive *JKRArchive::Mount(const char *path, u32 /* mountMode */, JKRHeap *heap,
        u32 mountDirection) {
    return REPLACED(Mount)(path, MountMode::Mem, heap, mountDirection);
}
