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

bool JFWDisplay::ensureIn(s32 duration) {
    u32 faderStatus = getFaderStatus();
    switch (faderStatus) {
    case JUTFader::Status::Out:
    case JUTFader::Status::FadingOut:
        startFadeIn(duration);
        return false;
    case JUTFader::Status::In:
        return true;
    default:
        return false;
    }
}

bool JFWDisplay::ensureOut(s32 duration) {
    u32 faderStatus = getFaderStatus();
    switch (faderStatus) {
    case JUTFader::Status::Out:
        return true;
    case JUTFader::Status::In:
    case JUTFader::Status::FadingIn:
        startFadeOut(duration);
        return false;
    default:
        return false;
    }
}
