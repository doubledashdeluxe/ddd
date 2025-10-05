#pragma once

#include "portable/Algorithm.hh"

template <typename T>
class UniquePtr {
public:
    UniquePtr(T *ptr = nullptr) : m_ptr(ptr) {}

    ~UniquePtr() {
        delete m_ptr;
    }

    EXPLICIT operator bool() const {
        return m_ptr;
    }

    T &operator*() const {
        return *m_ptr;
    }

    T *operator->() const {
        return m_ptr;
    }

    T *get() const {
        return m_ptr;
    }

    T *release() {
        T *ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

    void reset(T *ptr = nullptr) {
        delete m_ptr;
        m_ptr = ptr;
    }

private:
    UniquePtr(const UniquePtr &);
    UniquePtr &operator=(const UniquePtr &);

    T *m_ptr;
};

template <typename T>
class UniquePtr<T[]> {
public:
    UniquePtr(T *ptr = nullptr) : m_ptr(ptr) {}

    ~UniquePtr() {
        delete[] m_ptr;
    }

    EXPLICIT operator bool() const {
        return m_ptr;
    }

    T *get() const {
        return m_ptr;
    }

    T *release() {
        T *ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

    void reset(T *ptr = nullptr) {
        delete[] m_ptr;
        m_ptr = ptr;
    }

private:
    UniquePtr(const UniquePtr &);
    UniquePtr &operator=(const UniquePtr &);

    T *m_ptr;
};

template <typename T>
void Swap(UniquePtr<T> &a, UniquePtr<T> &b) {
    UniquePtr<T> tmp(a.release());
    a.reset(b.release());
    b.reset(tmp.release());
}
