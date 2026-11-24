#pragma once

#include <jsystem/JUTTexture.hh>

class GhostScreen {
public:
    GhostScreen();
    void reset();
    void setConsole(u32 index);
    bool hidden() const;
    void captureScene();
    void captureGhost();
    void draw();

private:
    static u32 X(u32 index);
    static u32 Y(u32 index);
    static u32 Width();
    static u32 Height();

    u16 m_x;
    u16 m_y;
    u16 m_width;
    u16 m_height;
    bool m_ready;
    bool m_hidden;
    u8 _0a[0x0c - 0x0a];
    JUTTexture m_sceneTexture;
    JUTTexture m_ghostTexture;
};
size_assert(GhostScreen, 0x8c);
