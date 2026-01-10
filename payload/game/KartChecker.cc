#include "KartChecker.hh"

bool KartChecker::lapRenewal() const {
    return m_lapRenewal;
}

bool KartChecker::raceEnd() const {
    return m_raceEnd;
}

s32 KartChecker::lap() const {
    return m_lap;
}
