#pragma once

#include <portable/Types.hh>

class CString {
public:
    static int Memcmp(const void *s1, const void *s2, size_t n);
    static void *Memcpy(void *dest, const void *src, size_t n);
    static void *Memmove(void *dest, const void *src, size_t n);
    static void *Memset(void *s, int c, size_t n);
    static char *Strchr(const char *s, int c);
    static int Strcmp(const char *s1, const char *s2);
    static size_t Strlen(const char *s);
    static int Strncmp(const char *s1, const char *s2, size_t n);
    static size_t Strnlen(const char *s, size_t maxlen);
    static char *Strrchr(const char *s, int c);

private:
    CString();
};
