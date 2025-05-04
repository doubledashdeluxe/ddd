#pragma once

#include <portable/Types.hh>

class CStrings {
public:
    static int Strncasecmp(const char *s1, const char *s2, size_t n);

private:
    CStrings();
};
