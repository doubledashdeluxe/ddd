#pragma once

#include <jsystem/J2DScreen.hh>

class OnlineBackground {
public:
    void draw(const J2DGraphContext *graphContext);
    void calc();

    static OnlineBackground *Create(JKRArchive *archive);
    static void Destroy();
    static OnlineBackground *Instance();

private:
    OnlineBackground(JKRArchive *archive);
    ~OnlineBackground();

    J2DScreen m_screen;
    J2DAnmBase *m_anmTransform;
    J2DAnmBase *m_anmTextureSRTKey;
    J2DAnmBase *m_anmColor;
    u8 m_anmTransformFrame;
    u16 m_anmTextureSRTKeyFrame;
    u8 m_anmColorFrame;

    static OnlineBackground *s_instance;
};
