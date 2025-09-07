#include "JSUList.hh"

JSUPtrLink *JSUPtrList::getFirstLink() const {
    return m_head;
}

JSUPtrLink *JSUPtrList::getLastLink() const {
    return m_tail;
}

u32 JSUPtrList::getNumLinks() const {
    return m_numLinks;
}

void *JSUPtrLink::getObject() const {
    return m_object;
}

JSUPtrList *JSUPtrLink::getList() const {
    return m_list;
}

JSUPtrLink *JSUPtrLink::getPrev() const {
    return m_prev;
}

JSUPtrLink *JSUPtrLink::getNext() const {
    return m_next;
}
