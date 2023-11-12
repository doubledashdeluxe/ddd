#include "JFWDisplay.hh"

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
