#include "KartInfo.hh"

const KartDB *KartInfo::getKartDB() const {
    return m_kartDB;
}

const CharDB *KartInfo::getCharDB(u32 characterIndex) const {
    return m_characters[characterIndex].charDB;
}
