#pragma once

#include <common/Types.hh>

template <typename T, size_t N>
class Array {
public:
    Array() {}

    Array(T value) {
        for (size_t index = 0; index < N; index++) {
            m_values[index] = value;
        }
    }

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
