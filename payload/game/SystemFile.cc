#include "SystemFile.hh"

#include "game/SystemRecord.hh"

void SystemFile::setPart(s32 /* filePart */) {
    m_filePart = 0;
}

void SystemFile::fetch() {
    REPLACED(fetch)();

    m_extendedSystemRecord = ExtendedSystemRecord::Instance();
}

void SystemFile::store() {
    REPLACED(store)();

    SystemRecord::Instance().applyAspectRatioSetting();

    ExtendedSystemRecord::Instance() = m_extendedSystemRecord;
}
