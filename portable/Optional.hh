#pragma once

#include "portable/New.hh"
#include "portable/Types.hh"

template <typename T>
class Optional {
public:
    Optional() : m_hasValue(false) {}

    Optional(const T &value) : m_hasValue(true) {
        new (&m_buffer) T(value);
    }

    ~Optional() {
        if (m_hasValue) {
            reinterpret_cast<T *>(m_buffer)->~T();
        }
    }

    Optional &operator=(const T &value) {
        new (&m_buffer) T(value);
        m_hasValue = true;
        return *this;
    }

    T &operator*() const {
        return *get();
    }

    T &operator*() {
        return *get();
    }

    T *operator->() {
        return get();
    }

    T *operator->() const {
        return get();
    }

    T *get() {
        return m_hasValue ? reinterpret_cast<T *>(m_buffer) : static_cast<T *>(nullptr);
    }

    const T *get() const {
        return m_hasValue ? reinterpret_cast<const T *>(m_buffer) : static_cast<T *>(nullptr);
    }

    void reset() {
        if (m_hasValue) {
            reinterpret_cast<T *>(m_buffer)->~T();
            m_hasValue = false;
        }
    }

    T &emplace() {
        new (&m_buffer) T;
        m_hasValue = true;
        return *reinterpret_cast<T *>(m_buffer);
    }

private:
    union {
        u8 m_buffer[sizeof(T)];
        Aligner<alignof(T)> m_aligner;
    };
    bool m_hasValue;
};
