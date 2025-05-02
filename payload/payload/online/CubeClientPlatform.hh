#include "payload/HeapAllocator.hh"

#include <jsystem/JKRHeap.hh>
#include <portable/online/ClientPlatform.hh>

class CubeClientPlatform : public ClientPlatform {
public:
    CubeClientPlatform(JKRHeap *heap);

    Allocator &allocator() override;
    Random &random() override;
    DNS &dns() override;

private:
    HeapAllocator m_allocator;
};
