#include "PerfOverlay.hh"

#include <cube/Clock.hh>
#include <cube/Memory.hh>
extern "C" {
#include <dolphin/OSTime.h>
#include <dolphin/VI.h>
}
#include <jsystem/J2DOrthoGraph.hh>
#include <jsystem/JKRExpHeap.hh>
#include <payload/Lock.hh>

void PerfOverlay::beginFrame() {
    m_drawBar.calc(*this);
    m_calcBar.calc(*this);

    for (u32 i = 0; i < m_threads.count(); i++) {
        if (m_threads[i] == m_mainThread) {
            m_threadColors[i] = (Color){255, 102, 102, 255};
        } else if (m_threads[i]) {
            m_threadColors[i] = (Color){255, 255, 255, 255};
        } else {
            m_threadColors[i] = (Color){0, 0, 0, 0};
        }
    }
    m_prevThread = OSGetCurrentThread();
    m_prevThreadIndex = 0;
    m_threads.fill(nullptr);

    m_renderBar.calc(*this);

    m_heaps.fill(nullptr);
    processHeap(JKRHeap::GetRootHeap());
    for (u32 i = 0, j = 0; i < m_heaps.count(); i++) {
        if (m_heaps[i]) {
            if (i != 0 && m_heaps[i] != m_heaps[i - 1]) {
                j ^= 1;
            }
            if (j == 0) {
                m_heapColors[i] = (Color){102, 255, 255, 255};
            } else {
                m_heapColors[i] = (Color){255, 255, 102, 255};
            }
        } else {
            m_heapColors[i] = (Color){0, 0, 0, 0};
        }
    }

    {
        Lock<NoInterrupts> lock;

        m_frameStart = OSGetSystemTime();
    }
}

void PerfOverlay::beginDraw() {
    m_drawBar.begin();
}

void PerfOverlay::beginRender() {
    m_renderBar.begin();
}

void PerfOverlay::endDraw() {
    m_drawBar.end();
}

void PerfOverlay::beginCalc() {
    m_calcBar.begin();
}

void PerfOverlay::endCalc() {
    m_calcBar.end();
}

void PerfOverlay::draw() {
    Color backgroundColor = {0, 0, 0, 102};
    FillBox(4, 426, Width, 6, backgroundColor);
    Color drawColor = {102, 255, 102, 255};
    m_drawBar.draw(427, drawColor);
    Color calcColor = {255, 102, 255, 255};
    m_calcBar.draw(427, calcColor);
    FillBoxes(429, m_threadColors);

    FillBox(4, 434, Width, 4, backgroundColor);
    Color renderColor = {102, 102, 255, 255};
    m_renderBar.draw(435, renderColor);

    FillBox(4, 440, Width, 4, backgroundColor);
    FillBoxes(441, m_heapColors);
}

void PerfOverlay::endRender() {
    m_renderBar.end();
}

void PerfOverlay::handleSwitchThread(OSThread *prev, OSThread *next) {
    u32 index = convertInstantToX(OSGetSystemTime());
    if (index > m_threads.count()) {
        return;
    }
    if (prev == m_prevThread) {
        for (u32 i = m_prevThreadIndex; i < index; i++) {
            m_threads[i] = prev;
        }
    }
    m_prevThread = next;
    m_prevThreadIndex = index;
}

void PerfOverlay::Init(JKRHeap *heap) {
    s_instance = new (heap, 0x4) PerfOverlay;
}

PerfOverlay *PerfOverlay::Instance() {
    return s_instance;
}

PerfOverlay::TimeBar::TimeBar() {}

void PerfOverlay::TimeBar::draw(s32 y, Color color) {
    FillBox(m_x, y, m_w, 2, color);
}

void PerfOverlay::TimeBar::calc(PerfOverlay &perfOverlay) {
    m_x = perfOverlay.convertInstantToX(m_start);
    m_w = perfOverlay.convertDurationToW(m_duration);
}

void PerfOverlay::TimeBar::begin() {
    m_start = OSGetSystemTime();
}

void PerfOverlay::TimeBar::end() {
    m_duration = OSGetSystemTime() - m_start;
}

PerfOverlay::PerfOverlay()
    : m_frameStart(OSGetSystemTime()), m_prevThread(nullptr), m_prevThreadIndex(0) {
    if (VIGetTvFormat() == VI_PAL) {
        m_frameDuration = Clock::MicrosecondsToTicks(20000);
    } else {
        m_frameDuration = Clock::MicrosecondsToTicks(16683);
    }
    m_mainThread = OSGetCurrentThread();
}

s32 PerfOverlay::convertInstantToX(s64 instant) {
    return 4 + convertDurationToW(instant - m_frameStart);
}

s32 PerfOverlay::convertDurationToW(s64 duration) {
    return Width * duration / m_frameDuration;
}

void PerfOverlay::processHeap(JKRHeap *heap) {
    s32 x0 = ConvertPtrToX(heap->getStartAddr());
    s32 x1 = ConvertPtrToX(heap->getEndAddr());
    fillHeaps(x0, x1, heap);

    JKRExpHeap *expHeap = heap->downcast<JKRExpHeap>();
    if (expHeap) {
        Lock<Mutex> lock(expHeap->getMutex());

        for (const JKRExpHeap::MemBlock *block = expHeap->getFreeListHead(); block;
                block = block->getNext()) {
            s32 x0 = ConvertPtrToX(block);
            s32 x1 = ConvertPtrToX(reinterpret_cast<const u8 *>(block) + block->getSize());
            fillHeaps(x0, x1, nullptr);
        }
    }

    const JSUTree<JKRHeap> &tree = heap->getHeapTree();
    for (JSUTree<JKRHeap> *child = tree.getFirstChild(); child != tree.getEndChild();
            child = child->getNextChild()) {
        processHeap(child->getObject());
    }
}

void PerfOverlay::fillHeaps(s32 x0, s32 x1, JKRHeap *heap) {
    for (s32 x = x0; x < x1; x++) {
        if (x >= 0 && x < Width) {
            m_heaps[x] = heap;
        }
    }
}

s32 PerfOverlay::ConvertPtrToX(const void *ptr) {
    u32 address = Memory::CachedToPhysical(ptr);
    return address / (24 * 1024 * 1024 / Width);
}

void PerfOverlay::FillBox(s32 x, s32 y, s32 w, s32 h, Color color) {
    J2DFillBox(x, y, w, h, (GXColor){color.r, color.g, color.b, color.a});
}

void PerfOverlay::FillBoxes(s32 y, Array<Color, Width> &colors) {
    s32 start = 0;
    for (u32 i = 0; i <= colors.count(); i++) {
        if (i == colors.count() || colors[i] != colors[start]) {
            s32 width = i - start;
            if (width > 0 && colors[start].a > 0) {
                FillBox(4 + start, y, width, 2, colors[start]);
            }
            start = i;
        }
    }
}

PerfOverlay *PerfOverlay::s_instance;
