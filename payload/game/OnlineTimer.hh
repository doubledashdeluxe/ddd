#pragma once

#include <jsystem/J2DScreen.hh>

class OnlineTimer {
public:
    void init(u32 duration);
    void draw(const J2DGraphContext *graphContext);
    void calc();
    void setAlpha(u8 alpha);
    bool hasExpired() const;

    static OnlineTimer *Create(JKRArchive *archive);
    static void Destroy();
    static OnlineTimer *Instance();

private:
    OnlineTimer(JKRArchive *archive);
    ~OnlineTimer();

    u32 m_duration;
    u32 m_frame;
    J2DScreen m_screen;
    u8 m_alpha;

    static OnlineTimer *s_instance;
};
