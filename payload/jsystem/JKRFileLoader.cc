#include "JKRFileLoader.hh"

s32 JKRFileLoader::GetGlbResSize(const void *ptr, JKRFileLoader *fileLoader) {
    if (fileLoader) {
        return fileLoader->getResSize(ptr);
    }

    for (JSUPtrLink *link = s_volumeList.getFirstLink(); link; link = link->getNext()) {
        fileLoader = reinterpret_cast<JKRFileLoader *>(link->getObjectPtr());
        s32 resSize = fileLoader->getResSize(ptr);
        if (resSize != -1) {
            return resSize;
        }
    }
    return -1;
}
