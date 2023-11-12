#pragma once

#include <common/Types.hh>

class ResMgr {
public:
    static void LoadKeepData();
    static void LoadCourseData(u32 courseId, u32 courseOrder);

private:
    ResMgr();
};
size_assert(ResMgr, 0x1);
