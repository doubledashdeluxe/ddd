#pragma once

#include <portable/Types.hh>

class JSUPtrLink;

class JSUPtrList {
public:
    JSUPtrList();
    ~JSUPtrList();

    bool append(JSUPtrLink *link);
    bool prepend(JSUPtrLink *link);

    JSUPtrLink *getFirstLink() const;
    JSUPtrLink *getLastLink() const;
    u32 getNumLinks() const;

private:
    JSUPtrLink *m_head;
    JSUPtrLink *m_tail;
    u32 m_numLinks;
};
size_assert(JSUPtrList, 0xc);

class JSUPtrLink {
public:
    JSUPtrLink(void *data);
    ~JSUPtrLink();

    void *getObject() const;
    JSUPtrList *getList() const;
    JSUPtrLink *getPrev() const;
    JSUPtrLink *getNext() const;

private:
    void *m_object;
    JSUPtrList *m_list;
    JSUPtrLink *m_prev;
    JSUPtrLink *m_next;
};
size_assert(JSUPtrLink, 0x10);

template <typename T>
class JSULink;

template <typename T>
class JSUList : public JSUPtrList {
public:
    JSULink<T> *getFirstLink() const {
        return static_cast<JSULink<T> *>(JSUPtrList::getFirstLink());
    }

    JSULink<T> *getLastLink() const {
        return static_cast<JSULink<T> *>(JSUPtrList::getLastLink());
    }
};

template <typename T>
class JSULink : public JSUPtrLink {
public:
    T *getObject() const {
        return static_cast<T *>(JSUPtrLink::getObject());
    }

    JSUList<T> *getList() const {
        return static_cast<JSUList<T> *>(JSUPtrLink::getList());
    }

    JSULink<T> *getPrev() const {
        return static_cast<JSULink *>(JSUPtrLink::getPrev());
    }

    JSULink<T> *getNext() const {
        return static_cast<JSULink *>(JSUPtrLink::getNext());
    }
};
