#pragma once

#ifdef CUBE
#include "portable/Types.hh"

void *operator new(size_t size, void *ptr);
#else
#include <new>
#endif
