#include "SystemFile.hh"

#include "game/SystemRecord.hh"

void SystemFile::store() {
    REPLACED(store)();

    SystemRecord::Instance().applyAspectRatioSetting();
}
