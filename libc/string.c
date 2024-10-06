#include <string.h>

size_t strnlen(const char *s, size_t maxlen) {
    for (size_t i = 0; i < maxlen; i++) {
        if (s[i] == '\0') {
            return i;
        }
    }
    return maxlen;
}
