#pragma once

#include <common/Types.hh>

template <typename T, size_t N>
class Array {
public:
    Array() {}

    Array(T value) {
        fill(value);
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

    void fill(T value) {
        for (size_t index = 0; index < N; index++) {
            m_values[index] = value;
        }
    }

    void rotateLeft(size_t amount) {
        while (amount--) {
            T value = m_values[0];
            for (size_t index = 0; index < N - 1; index++) {
                m_values[index] = m_values[index + 1];
            }
            m_values[N - 1] = value;
        }
    }

    void rotateRight(size_t amount) {
        while (amount--) {
            T value = m_values[N - 1];
            for (size_t index = N - 1; index > 0; index--) {
                m_values[index] = m_values[index - 1];
            }
            m_values[0] = value;
        }
    }

private:
    T m_values[N];
};
