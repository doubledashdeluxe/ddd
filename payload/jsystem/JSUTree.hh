#pragma once

#include "jsystem/JSUList.hh"

template <typename T>
class JSUTree
    : public JSUList<T>
    , public JSULink<T> {
public:
    JSUTree(T *value) : JSUList<T>(), JSULink<T>(value) {}

    ~JSUTree() {}

    bool appendChild(JSUTree<T> *child) {
        return append(child);
    }

    bool prependChild(JSUTree<T> *child) {
        return prepend(child);
    }

    bool removeChild(JSUTree<T> *child) {
        return remove(child);
    }

    bool insertChild(JSUTree<T> *before, JSUTree<T> *child) {
        return insert(before, child);
    }

    JSUTree<T> *getEndChild() const {
        return nullptr;
    }

    JSUTree<T> *getFirstChild() const {
        return static_cast<JSUTree<T> *>(JSUList<T>::getFirstLink());
    }

    JSUTree<T> *getLastChild() const {
        return static_cast<JSUTree<T> *>(JSUList<T>::getLastLink());
    }

    JSUTree<T> *getPrevChild() const {
        return static_cast<JSUTree<T> *>(JSULink<T>::getPrev());
    }

    JSUTree<T> *getNextChild() const {
        return static_cast<JSUTree<T> *>(JSULink<T>::getNext());
    }

    u32 getNumChildren() const {
        return JSUPtrList::getNumLinks();
    }

    T *getObject() const {
        return JSULink<T>::getObject();
    }

    JSUTree<T> *getParent() const {
        return JSULink<T>::getList();
    }
};
