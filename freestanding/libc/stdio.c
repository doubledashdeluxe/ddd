#include <stdio.h>

#include <lwprintf/lwprintf.h>

int snprintf(char *str, size_t size, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int result = vsnprintf(str, size, format, ap);
    va_end(ap);
    return result;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    return lwprintf_vsnprintf(str, size, format, ap);
}
