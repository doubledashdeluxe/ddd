#pragma once

#include <common/Types.h>

int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dest, const void *src, size_t count);
void *memset(void *s, int c, size_t n);
size_t strlcpy(char *dst, const char *src, size_t size);
size_t strlen(const char *s);
int strncmp(const char *s1, const char *s2, size_t n);
char *strrchr(const char *s, int c);
