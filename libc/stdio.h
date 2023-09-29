#pragma once

#include <stdarg.h>

int snprintf(char *str, size_t size, const char *format, ...);
int vprintf(const char *format, va_list ap);
