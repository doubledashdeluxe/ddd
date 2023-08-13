#pragma once

#include <common/Types.h>

void *memcpy(void *dest, const void *src, size_t count);
void *memset(void *s, int c, size_t n);
size_t strlcpy(char *dst, const char *src, size_t size);
size_t strlen(const char *s);
