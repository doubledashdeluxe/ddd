#pragma once

#include <common/Types.hh>

class JSUPtrLink;

class JSUPtrList {
public:
    JSUPtrList();
    ~JSUPtrList();

    bool append(JSUPtrLink *link);
    bool prepend(JSUPtrLink *link);

    JSUPtrLink *getFirstLink() const;
    JSUPtrLink *getLastLink() const;

private:
    JSUPtrLink *m_head;
    JSUPtrLink *m_tail;
    u32 m_linkCount;
};
size_assert(JSUPtrList, 0xc);

class JSUPtrLink {
public:
    JSUPtrLink(void *data);
    ~JSUPtrLink();

    void *getObjectPtr() const;
    JSUPtrList *getList() const;
    JSUPtrLink *getNext() const;
    JSUPtrLink *getPrev() const;

private:
    void *m_value;
    JSUPtrList *m_list;
    JSUPtrLink *m_prev;
    JSUPtrLink *m_next;
};
size_assert(JSUPtrLink, 0x10);
