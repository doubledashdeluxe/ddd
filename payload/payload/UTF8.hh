#pragma once

#include <portable/Types.hh>

class UTF8 {
public:
    static u8 ToAscii(u32 c);
    static u32 ToLower(u32 c);
    static u32 Length(const char *s);
    static u32 Next(const char *&s);
    static s32 Compare(const char *s1, const char *s2);
    static s32 CaseCompare(const char *s1, const char *s2);

private:
    UTF8();
};
