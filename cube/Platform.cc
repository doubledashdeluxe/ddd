#include "Platform.hh"

#include "cube/Dolphin.hh"
#include "cube/ios/Resource.hh"

extern "C" u32 consoleType;

bool Platform::IsGameCube() {
    if (!s_gameCubeDetectionIsDone) {
        DetectGameCube();
    }

    return s_isGameCube;
}

bool Platform::IsDolphin() {
    if (!s_dolphinDetectionIsDone) {
        DetectDolphin();
    }

    return s_isDolphin;
}

bool Platform::GetDolphinVersion(DolphinVersion &dolphinVersion) {
    if (!s_dolphinDetectionIsDone) {
        DetectDolphin();
    }

    if (s_hasDolphinVersion) {
        dolphinVersion = s_dolphinVersion;
    }
    return s_hasDolphinVersion;
}

void Platform::DetectGameCube() {
    if (consoleType == 0) {
        consoleType = 0x23;
    }
    s_isGameCube = (consoleType & 0x0fffffff) < 0x8;

    s_gameCubeDetectionIsDone = true;
}

void Platform::DetectDolphin() {
    if (!IsGameCube()) {
        Dolphin dolphin;
        if (dolphin.ok()) {
            // Modern versions
            s_isDolphin = true;
            s_hasDolphinVersion = dolphin.getVersion(s_dolphinVersion);
        } else {
            IOS::Resource sha("/dev/sha", IOS::Mode::None);
            if (!sha.ok()) {
                // Old versions
                s_isDolphin = true;
            }
        }
    }

    s_dolphinDetectionIsDone = true;
}

bool Platform::s_gameCubeDetectionIsDone = false;
bool Platform::s_dolphinDetectionIsDone = false;
bool Platform::s_isGameCube = false;
bool Platform::s_isDolphin = false;
bool Platform::s_hasDolphinVersion = false;
DolphinVersion Platform::s_dolphinVersion;
