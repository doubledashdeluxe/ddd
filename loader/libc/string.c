#include <string.h>

void *memcpy(void *dest, const void *src, size_t n) {
    u8 *d = dest;
    const u8 *s = src;

    while (n-- > 0) {
        *d++ = *s++;
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    u8 *p = s;
    for (size_t i = 0; i < n; i++) {
        p[i] = c;
    }
    return s;
}

size_t strlen(const char *s) {
    const char *f = s;
    while (*s != '\0') {
        s++;
    }
    return s - f;
}
