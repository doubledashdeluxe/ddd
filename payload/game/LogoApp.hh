#pragma once

#include "game/GameApp.hh"

#include <payload/Replace.hh>

class LogoApp : public GameApp {
public:
    ~LogoApp() override;
    REPLACE void draw() override;
    REPLACE void calc() override;
    void reset() override;

    REPLACE static LogoApp *Create();

private:
    typedef void (LogoApp::*State)();

    LogoApp();

    u8 _0c[0x3c - 0x0c];
    u32 m_state;

    static LogoApp *s_instance;
};
size_assert(LogoApp, 0x3c + sizeof(u32));
