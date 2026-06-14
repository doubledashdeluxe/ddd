#pragma once

#include "portable/Algorithm.hh"
#include "portable/New.hh"

template <typename T, size_t N>
class Ring {
public:
    Ring() : m_front(0), m_count(0) {}

    ~Ring() {
        reset();
    }

    bool empty() const {
        return m_count == 0;
    }

    bool full() const {
        return m_count == N;
    }

    size_t count() const {
        return m_count;
    }

    const T *front() const {
        if (empty()) {
            return nullptr;
        }

        return &(*this)[0];
    }

    T *front() {
        if (empty()) {
            return nullptr;
        }

        return &(*this)[0];
    }

    const T *back() const {
        if (empty()) {
            return nullptr;
        }

        return &(*this)[m_count - 1];
    }

    T *back() {
        if (empty()) {
            return nullptr;
        }

        return &(*this)[m_count - 1];
    }

    const T &operator[](size_t index) const {
        return reinterpret_cast<const T *>(m_buffer)[(m_front + index) % N];
    }

    T &operator[](size_t index) {
        return reinterpret_cast<T *>(m_buffer)[(m_front + index) % N];
    }

    T *emplaceFront() {
        if (full()) {
            return nullptr;
        }

        m_front = (m_front - 1) % N;
        new (&reinterpret_cast<T *>(m_buffer)[m_front]) T;
        m_count++;
        return &(*this)[0];
    }

    T *emplaceBack() {
        if (full()) {
            return nullptr;
        }

        new (&reinterpret_cast<T *>(m_buffer)[(m_front + m_count) % N]) T;
        m_count++;
        return &(*this)[m_count - 1];
    }

    T *pushFront(T value) {
        if (full()) {
            return nullptr;
        }

        m_front = (m_front - 1) % N;
        new (&reinterpret_cast<T *>(m_buffer)[m_front]) T(value);
        m_count++;
        return &(*this)[0];
    }

    T *pushBack(T value) {
        if (full()) {
            return nullptr;
        }

        new (&reinterpret_cast<T *>(m_buffer)[(m_front + m_count) % N]) T(value);
        m_count++;
        return &(*this)[m_count - 1];
    }

    bool popFront() {
        if (empty()) {
            return false;
        }

        reinterpret_cast<T *>(m_buffer)[m_front].~T();
        m_front = (m_front + 1) % N;
        m_count--;
        return true;
    }

    bool popBack() {
        if (empty()) {
            return false;
        }

        reinterpret_cast<T *>(m_buffer)[(m_front + m_count - 1) % N].~T();
        m_count--;
        return true;
    }

    void swapRemoveFront(size_t index) {
        Swap((*this)[index], (*this)[0]);
        popFront();
    }

    void swapRemoveBack(size_t index) {
        Swap((*this)[index], (*this)[m_count - 1]);
        popBack();
    }

    void reset() {
        while (popFront()) {}
    }

private:
    union {
        u8 m_buffer[sizeof(T[N])];
        Aligner<alignof(T[N])> m_aligner;
    };
    size_t m_front;
    size_t m_count;
};
