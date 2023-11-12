#include "J2DAnmLoaderDataBase.hh"

J2DAnmBase *J2DAnmLoaderDataBase::Load(const char *name, JKRFileLoader *fileLoader) {
    void *resource = JKRFileLoader::GetGlbResource(name, fileLoader);
    return Load(resource);
}
