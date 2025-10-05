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
        reset();
        new (&m_buffer) T(value);
        m_hasValue = true;
        return *this;
    }

    EXPLICIT operator bool() const {
        return m_hasValue;
    }

    const T &operator*() const {
        return *value();
    }

    T &operator*() {
        return *value();
    }

    const T *operator->() const {
        return value();
    }

    T *operator->() {
        return value();
    }

    const T *get() const {
        return m_hasValue ? value() : Nothing();
    }

    T *get() {
        return m_hasValue ? value() : Nothing();
    }

    void reset() {
        if (m_hasValue) {
            reinterpret_cast<T *>(m_buffer)->~T();
            m_hasValue = false;
        }
    }

    T &emplace() {
        reset();
        new (&m_buffer) T;
        m_hasValue = true;
        return *reinterpret_cast<T *>(m_buffer);
    }

    T &getOrEmplace() {
        return m_hasValue ? *value() : emplace();
    }

private:
    const T *value() const {
        return reinterpret_cast<const T *>(m_buffer);
    }

    T *value() {
        return reinterpret_cast<T *>(m_buffer);
    }

    static T *Nothing() {
        return nullptr;
    }

    union {
        u8 m_buffer[sizeof(T)];
        Aligner<alignof(T)> m_aligner;
    };
    bool m_hasValue;
};

template <typename T>
bool operator==(const Optional<T> &a, const Optional<T> &b) {
    return (!a && !b) || (a && b && *a == *b);
}

template <typename T>
bool operator!=(const Optional<T> &a, const Optional<T> &b) {
    return !(a == b);
}

template <typename T, typename U>
bool operator==(const Optional<T> &a, const U &b) {
    return a && *a == b;
}

template <typename T, typename U>
bool operator!=(const Optional<T> &a, const U &b) {
    return !(a == b);
}
