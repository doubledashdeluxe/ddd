#pragma once

#include <portable/Allocator.hh>

class NativeAllocator final : public Allocator {
public:
    NativeAllocator();
    ~NativeAllocator();

    void *alloc(size_t size) override;
};
