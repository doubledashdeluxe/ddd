#include "CString.hh"

int CString::Memcmp(const void *s1, const void *s2, size_t n) {
    const u8 *p1 = static_cast<const u8 *>(s1);
    const u8 *p2 = static_cast<const u8 *>(s2);
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

void *CString::Memcpy(void *dest, const void *src, size_t n) {
    u8 *d = static_cast<u8 *>(dest);
    const u8 *s = static_cast<const u8 *>(src);
    while (n-- > 0) {
        *d++ = *s++;
    }
    return dest;
}

void *CString::Memmove(void *dest, const void *src, size_t n) {
    if (dest <= src) {
        return Memcpy(dest, src, n);
    }

    u8 *d = static_cast<u8 *>(dest);
    const u8 *s = static_cast<const u8 *>(src);
    while (n-- > 0) {
        d[n] = s[n];
    }
    return dest;
}

void *CString::Memset(void *s, int c, size_t n) {
    u8 *p = static_cast<u8 *>(s);
    for (size_t i = 0; i < n; i++) {
        p[i] = c;
    }
    return s;
}

char *CString::Strchr(const char *s, int c) {
    do {
        if (*s == c) {
            return const_cast<char *>(s);
        }
    } while (*s++ != '\0');
    return nullptr;
}

int CString::Strcmp(const char *s1, const char *s2) {
    for (; *s1 == *s2; s1++, s2++) {
        if (*s1 == '\0') {
            return 0;
        }
    }
    return *s1 < *s2 ? -1 : 1;
}

size_t CString::Strlen(const char *s) {
    const char *f = s;
    while (*s != '\0') {
        s++;
    }
    return s - f;
}

int CString::Strncmp(const char *s1, const char *s2, size_t n) {
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

size_t CString::Strnlen(const char *s, size_t maxlen) {
    for (size_t i = 0; i < maxlen; i++) {
        if (s[i] == '\0') {
            return i;
        }
    }
    return maxlen;
}

char *CString::Strrchr(const char *s, int c) {
    char *p = nullptr;
    do {
        if (*s == c) {
            p = const_cast<char *>(s);
        }
    } while (*(s++));
    return p;
}
