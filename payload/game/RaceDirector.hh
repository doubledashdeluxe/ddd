#pragma once

#include <common/Types.hh>
#include <payload/Replace.hh>

class RaceDirector {
private:
    class Race2DMode {
    public:
        enum {
            OneTwo = 0,
            ThreeFour = 1,
        };

    private:
        Race2DMode();
    };

    void REPLACED(resetCommon)();
    REPLACE void resetCommon();

    u8 _00[0x04 - 0x00];
    u32 m_race2DMode;
    u8 _08[0x48 - 0x08];
};
size_assert(RaceDirector, 0x48);
