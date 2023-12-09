#include "JSUList.hh"

JSUPtrLink *JSUPtrList::getFirstLink() const {
    return m_head;
}

JSUPtrLink *JSUPtrList::getLastLink() const {
    return m_tail;
}

void *JSUPtrLink::getObjectPtr() const {
    return m_value;
}

JSUPtrList *JSUPtrLink::getList() const {
    return m_list;
}

JSUPtrLink *JSUPtrLink::getNext() const {
    return m_next;
}

JSUPtrLink *JSUPtrLink::getPrev() const {
    return m_prev;
}
