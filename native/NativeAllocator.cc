#include "NativeAllocator.hh"

NativeAllocator::NativeAllocator() = default;

NativeAllocator::~NativeAllocator() = default;

void *NativeAllocator::alloc(size_t size) {
    return operator new(size);
}
