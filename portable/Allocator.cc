#include "Allocator.hh"

void *operator new(size_t size, Allocator &allocator) {
    return allocator.alloc(size);
}
