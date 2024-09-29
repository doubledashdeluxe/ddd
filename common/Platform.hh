#pragma once

#include "common/Dolphin.hh"

class Platform {
public:
    static bool IsGameCube();
    static bool IsDolphin();
    static bool GetDolphinVersion(Dolphin::Version &dolphinVersion);

private:
    Platform();

    static void DetectGameCube();
    static void DetectDolphin();

    static bool s_gameCubeDetectionIsDone;
    static bool s_dolphinDetectionIsDone;
    static bool s_isGameCube;
    static bool s_isDolphin;
    static bool s_hasDolphinVersion;
    static Dolphin::Version s_dolphinVersion;
};
