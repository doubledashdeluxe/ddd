#pragma once

#ifdef __CWCC__
#include <common/Types.hh>

void *operator new(size_t size, void *ptr);
#else
#include <new>
#endif
