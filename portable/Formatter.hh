#pragma once

#include "portable/Array.hh"

class Formatter {
public:
    Formatter(char *string, size_t size);

    template <size_t N>
    Formatter(Array<char, N> &array) : m_string(array.values()), m_size(array.count()) {}

    ~Formatter();
    void printf(const char *format, ...);

private:
    char *m_string;
    size_t m_size;
};
