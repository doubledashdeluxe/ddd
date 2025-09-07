#pragma once

#include <jsystem/JSUTree.hh>
#include <payload/Mutex.hh>
#include <portable/Types.hh>

class JKRHeap {
public:
    virtual ~JKRHeap();
    virtual void vf_0c();
    virtual u32 getHeapType() const = 0;
    // ...

    template <typename H>
    const H *downcast() const {
        return static_cast<const H *>(getHeapType() == H::GetHeapType() ? this : nullptr);
    }

    template <typename H>
    H *downcast() {
        return static_cast<H *>(getHeapType() == H::GetHeapType() ? this : nullptr);
    }

    void becomeCurrentHeap();
    void destroy();
    void *alloc(u32 size, s32 align);
    void free(void *ptr);
    void freeAll();
    s32 resize(void *ptr, u32 size);
    u32 getFreeSize();
    u32 getMaxAllocatableSize(s32 align);

    Mutex &getMutex();
    void *getStartAddr() const;
    void *getEndAddr() const;
    const JSUTree<JKRHeap> &getHeapTree() const;

    static JKRHeap *GetSystemHeap();
    static JKRHeap *GetCurrentHeap();
    static JKRHeap *GetRootHeap();
    static JKRHeap *FindFromRoot(void *ptr);

private:
    u8 _04[0x18 - 0x04];
    Mutex m_mutex;
    void *m_startAddr;
    void *m_endAddr;
    u8 _38[0x40 - 0x38];
    JSUTree<JKRHeap> m_heapTree;
    u8 _5c[0x6c - 0x5c];

    static JKRHeap *s_systemHeap;
    static JKRHeap *s_currentHeap;
    static JKRHeap *s_rootHeap;
};
size_assert(JKRHeap, 0x6c);

void *operator new(size_t size, JKRHeap *heap, s32 alignment);
void *operator new[](size_t size, JKRHeap *heap, s32 alignment);
