#pragma once

#include <common/Types.hh>

template <typename T, size_t N>
class Array {
public:
    size_t count() const {
        return N;
    }

    const T *values() const {
        return m_values;
    }

    T *values() {
        return m_values;
    }

    const T &operator[](size_t index) const {
        return m_values[index];
    }

    T &operator[](size_t index) {
        return m_values[index];
    }

private:
    T m_values[N];
};
