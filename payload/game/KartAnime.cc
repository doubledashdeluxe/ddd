#include "KartAnime.hh"

bool KartAnime::isChangeStart() const {
    return m_isChangeStart;
}

void KartAnime::makeCrouchAnime() {
    m_isCrouch = true;
}
