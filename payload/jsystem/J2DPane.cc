#include "J2DPane.hh"

#include <common/Array.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
}

const TBox<f32> &J2DPane::getGlobalBox() const {
    return m_globalBox;
}

J2DPane *J2DPane::search(const char *format, ...) {
    va_list vlist;
    va_start(vlist, format);
    J2DPane *pane = vsearch(format, vlist);
    va_end(vlist);
    return pane;
}

J2DPane *J2DPane::vsearch(const char *format, va_list vlist) {
    Array<char, 9> tag;
    int length = vsnprintf(tag.values(), tag.count(), format, vlist);

    u64 uTag = 0;
    memcpy(&uTag, tag.values(), sizeof(uTag));
    uTag >>= (8 - length) * 8;
    return search(uTag);
}

void J2DPane::setHasARScissor(bool hasARScissor, bool recursive) {
    m_hasARScissor = hasARScissor;

    set(&J2DPane::setHasARScissor, hasARScissor, recursive);
}

void J2DPane::setHasARTrans(bool hasARTrans, bool recursive) {
    m_hasARTrans = hasARTrans;

    set(&J2DPane::setHasARTrans, hasARTrans, recursive);
}

void J2DPane::setHasARShift(bool hasARShift, bool recursive) {
    m_hasARShift = hasARShift;

    set(&J2DPane::setHasARShift, hasARShift, recursive);
}

void J2DPane::setHasARShiftLeft(bool hasARShiftLeft, bool recursive) {
    m_hasARShiftLeft = hasARShiftLeft;

    set(&J2DPane::setHasARShiftLeft, hasARShiftLeft, recursive);
}

void J2DPane::setHasARShiftRight(bool hasARShiftRight, bool recursive) {
    m_hasARShiftRight = hasARShiftRight;

    set(&J2DPane::setHasARShiftRight, hasARShiftRight, recursive);
}

void J2DPane::setHasARScale(bool hasARScale, bool recursive) {
    m_hasARScale = hasARScale;

    set(&J2DPane::setHasARScale, hasARScale, recursive);
}

void J2DPane::setHasARTexCoords(bool hasARTexCoords, bool recursive) {
    m_hasARTexCoords = hasARTexCoords;

    set(&J2DPane::setHasARTexCoords, hasARTexCoords, recursive);
}

s32 J2DPane::GetARShift() {
    return s_arShift;
}

f32 J2DPane::GetARScale() {
    return s_arScale;
}

void J2DPane::SetAspectRatio(f32 aspectRatio) {
    s_aspectRatio = aspectRatio;
    s_arWidth = aspectRatio / (19.0f / 14.0f) * 608.0f;
    s_arShift = (static_cast<s32>(s_arWidth) - 608) / 2;
    s_arScale = 608.0f / s_arWidth;
}

void J2DPane::changeUseTrans(J2DPane *parent) {
    REPLACED(changeUseTrans)(parent);

    m_hasARScissor = false;
    m_hasARTrans = true;
    m_hasARShift = true;
    m_hasARShiftLeft = false;
    m_hasARShiftRight = false;
    m_hasARScale = true;
    m_hasARTexCoords = true;
}

void J2DPane::set(BoolSetter setter, bool value, bool recursive) {
    if (!recursive) {
        return;
    }

    for (J2DPane *pane = getFirstChildPane(); pane; pane = pane->getNextChildPane()) {
        (pane->*setter)(value, recursive);
    }
}

f32 J2DPane::s_aspectRatio = 19.0f / 14.0f;
u32 J2DPane::s_arWidth = 608;
s32 J2DPane::s_arShift = 0;
f32 J2DPane::s_arScale = 1.0f;
