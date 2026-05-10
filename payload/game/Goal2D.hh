#pragma once

#include <jsystem/J2DOrthoGraph.hh>
#include <payload/Replace.hh>

class Goal2D {
public:
    void REPLACED(init)();
    REPLACE void init();

    static bool End();

private:
    u8 _00[0x04 - 0x00];
    J2DOrthoGraph *m_orthoGraphs[4];
    u8 _14[0xec - 0x14];

    static bool s_end;
};
size_assert(Goal2D, 0xec);
