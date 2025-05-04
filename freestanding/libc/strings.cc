extern "C" {
#include <strings.h>
}

#include <portable/CStrings.hh>

int strncasecmp(const char *s1, const char *s2, size_t n) {
    return CStrings::Strncasecmp(s1, s2, n);
}
