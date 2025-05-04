extern "C" {
#include <string.h>
}

#include <portable/CString.hh>

int memcmp(const void *s1, const void *s2, size_t n) {
    return CString::Memcmp(s1, s2, n);
}

void *memcpy(void *dest, const void *src, size_t n) {
    return CString::Memcpy(dest, src, n);
}

void *memmove(void *dest, const void *src, size_t n) {
    return CString::Memmove(dest, src, n);
}

void *memset(void *s, int c, size_t n) {
    return CString::Memset(s, c, n);
}

char *strchr(const char *s, int c) {
    return CString::Strchr(s, c);
}

int strcmp(const char *s1, const char *s2) {
    return CString::Strcmp(s1, s2);
}

size_t strlen(const char *s) {
    return CString::Strlen(s);
}

int strncmp(const char *s1, const char *s2, size_t n) {
    return CString::Strncmp(s1, s2, n);
}

char *strrchr(const char *s, int c) {
    return CString::Strrchr(s, c);
}
