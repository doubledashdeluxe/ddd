#include "JUTGamePad.hh"

u32 JUTGamePad::CButton::level() const {
    return m_level;
}

u32 JUTGamePad::CButton::risingEdge() const {
    return m_risingEdge;
}

u32 JUTGamePad::CButton::fallingEdge() const {
    return m_fallingEdge;
}

u32 JUTGamePad::CButton::repeat() const {
    return m_repeat;
}

const JUTGamePad::CButton &JUTGamePad::button() const {
    return m_button;
}
