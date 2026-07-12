#pragma once

#include <formats/Online.hh>
#include <portable/DolphinVersion.hh>

class Platform {
public:
    typedef Array<char, MaxPlatformLength + 1> String;

    static bool IsGameCube();
    static bool IsDolphin();
    static bool GetDolphinVersion(DolphinVersion &dolphinVersion);
    static String GetString();

    static u32 s_iosVersion;

private:
    Platform();

    static void DetectGameCube();
    static void DetectDolphin();

    static bool s_gameCubeDetectionIsDone;
    static bool s_dolphinDetectionIsDone;
    static bool s_isGameCube;
    static bool s_isDolphin;
    static bool s_hasDolphinVersion;
    static DolphinVersion s_dolphinVersion;
    static String s_string;
};
