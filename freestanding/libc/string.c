#include <string.h>

#include <portable/Types.h>

int memcmp(const void *s1, const void *s2, size_t n) {
    const u8 *p1 = s1, *p2 = s2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] < p2[i]) {
            return -1;
        }

        if (p1[i] > p2[i]) {
            return 1;
        }
    }

    return 0;
}

void *memcpy(void *dest, const void *src, size_t n) {
    u8 *d = dest;
    const u8 *s = src;
    while (n-- > 0) {
        *d++ = *s++;
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    if (dest <= src) {
        return memcpy(dest, src, n);
    }

    u8 *d = dest;
    const u8 *s = src;
    while (n-- > 0) {
        d[n] = s[n];
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

char *strchr(const char *s, int c) {
    do {
        if (*s == c) {
            return (char *)s;
        }
    } while (*s++ != '\0');
    return NULL;
}

int strcmp(const char *s1, const char *s2) {
    for (; *s1 == *s2; s1++, s2++) {
        if (*s1 == '\0') {
            return 0;
        }
    }
    return *s1 < *s2 ? -1 : 1;
}

size_t strlen(const char *s) {
    const char *f = s;
    while (*s != '\0') {
        s++;
    }
    return s - f;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (s1[i] < s2[i]) {
            return -1;
        }

        if (s1[i] > s2[i]) {
            return 1;
        }

        if (s1[i] == '\0') {
            return 0;
        }
    }

    return 0;
}

char *strrchr(const char *s, int c) {
    char *p = NULL;
    for (; *s; s++) {
        if (*s == c) {
            p = (char *)s;
        }
    }
    return p;
}
