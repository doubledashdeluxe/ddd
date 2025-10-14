#include "Formatter.hh"

extern "C" {
#include <stdarg.h>
#include <stdio.h>
}

Formatter::Formatter(char *string, size_t size) : m_string(string), m_size(size) {
    if (m_size > 0) {
        string[0] = '\0';
    }
}

Formatter::~Formatter() {}

void Formatter::printf(const char *format, ...) {
    va_list vlist;
    va_start(vlist, format);
    s32 size = vsnprintf(m_string, m_size, format, vlist);
    if (size >= static_cast<s32>(m_size)) {
        m_size = 0;
    } else if (size >= 0) {
        m_string += size;
        m_size -= size;
    }
    va_end(vlist);
}
