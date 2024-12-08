#pragma once

#include "jsystem/J2DGraphContext.hh"

class J2DOrthoGraph : public J2DGraphContext {
public:
    J2DOrthoGraph(f32 x, f32 y, f32 w, f32 h, f32 far, f32 near);
    ~J2DOrthoGraph() override;

    TBox<f32> m_orthoBox;

private:
    u8 _cc[0xd4 - 0xcc];
};
size_assert(J2DOrthoGraph, 0xd4);
