#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

class Course {
public:
    REPLACE u32 getCourseBGM() const;
    REPLACE u32 getFinalLapBGM() const;

private:
    u8 _000[0x53c - 0x000];
};
