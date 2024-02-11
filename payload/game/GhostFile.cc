#include "GhostFile.hh"

bool GhostFile::updateKartPadRecord(const KartPadRecord &padRecord) {
    if (m_ghostType < 0 || m_ghostType > 3) {
        return false;
    }

    if (padRecord.frame <= 0 || padRecord.frame >= 17864) {
        return false;
    }

    m_padRecord = padRecord;
    m_ghostType = 3;
    return true;
}
