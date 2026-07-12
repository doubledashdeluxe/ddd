#include "Platform.hh"

#include "cube/Dolphin.hh"
#include "cube/ios/Resource.hh"

extern "C" {
#include <stdio.h>
}

extern "C" u32 consoleType;

bool Platform::IsGameCube() {
    DetectGameCube();

    return s_isGameCube;
}

bool Platform::IsDolphin() {
    DetectDolphin();

    return s_isDolphin;
}

bool Platform::GetDolphinVersion(DolphinVersion &dolphinVersion) {
    DetectDolphin();

    if (s_hasDolphinVersion) {
        dolphinVersion = s_dolphinVersion;
    }
    return s_hasDolphinVersion;
}

Platform::String Platform::GetString() {
    DetectDolphin();

    return s_string;
}

u32 Platform::s_iosVersion;

void Platform::DetectGameCube() {
    if (s_gameCubeDetectionIsDone) {
        return;
    }

    if (consoleType == 0) {
        consoleType = 0x23;
    }
    s_isGameCube = (consoleType & 0x0fffffff) < 0x8;
    if (s_isGameCube) {
        snprintf(s_string.values(), s_string.count(), "GameCube");
    }

    s_gameCubeDetectionIsDone = true;
}

void Platform::DetectDolphin() {
    if (s_dolphinDetectionIsDone) {
        return;
    }

    if (!IsGameCube()) {
        Dolphin dolphin;
        Array<char, 64> versionString;
        if (dolphin.ok()) {
            // Modern versions
            s_isDolphin = true;
            s_hasDolphinVersion = dolphin.getVersion(versionString, s_dolphinVersion);
        } else {
            IOS::Resource sha("/dev/sha", IOS::Mode::None);
            if (!sha.ok()) {
                // Old versions
                s_isDolphin = true;
            } else {
                snprintf(s_string.values(), s_string.count(), "Wii");
            }
        }
        if (s_hasDolphinVersion) {
            snprintf(s_string.values(), s_string.count(), "Dolphin %s", versionString.values());
        } else {
            snprintf(s_string.values(), s_string.count(), "Dolphin");
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
Platform::String Platform::s_string;
