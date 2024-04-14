#pragma once

#include "jsystem/J2DGraphContext.hh"

class J2DOrthoGraph : public J2DGraphContext {
public:
    TBox<f32> m_orthoBox;

private:
    u8 _cc[0xd4 - 0xcc];
};
size_assert(J2DOrthoGraph, 0xd4);
