#include "New.hh"

#ifdef __CWCC__
void *operator new(size_t /* size */, void *ptr) {
    return ptr;
}
#endif
