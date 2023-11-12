#pragma once

#include "jsystem/J2DAnmBase.hh"
#include "jsystem/JKRFileLoader.hh"

class J2DAnmLoaderDataBase {
public:
    static J2DAnmBase *Load(const void *resource);
    static J2DAnmBase *Load(const char *name, JKRFileLoader *fileLoader);

    template <typename T>
    static T *Load(const char *name, JKRFileLoader *fileLoader) {
        void *resource = JKRFileLoader::GetGlbResource(name, fileLoader);
        return reinterpret_cast<T *>(Load(resource));
    }
};
