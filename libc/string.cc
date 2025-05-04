extern "C" {
#include <string.h>
}

#include <portable/CString.hh>

size_t strnlen(const char *s, size_t maxlen) {
    return CString::Strnlen(s, maxlen);
}
