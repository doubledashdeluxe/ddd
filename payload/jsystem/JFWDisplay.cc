#include "JFWDisplay.hh"

u32 JFWDisplay::getFaderStatus() const {
    if (!m_fader) {
        return JUTFader::Status::In;
    }

    return m_fader->getStatus();
}

void JFWDisplay::setFaderColor(u8 r, u8 g, u8 b) {
    m_fader->setColor(r, g, b);
}

bool JFWDisplay::startFadeIn(s32 duration) {
    if (!m_fader) {
        return true;
    }

    return m_fader->startFadeIn(duration);
}

bool JFWDisplay::startFadeOut(s32 duration) {
    if (!m_fader) {
        return true;
    }

    return m_fader->startFadeOut(duration);
}
