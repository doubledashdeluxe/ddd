#pragma once

#ifdef CUBE
#include "portable/Types.hh"

inline void *operator new(size_t /* size */, void *ptr) {
    return ptr;
}
#else
#include <new>
#endif
