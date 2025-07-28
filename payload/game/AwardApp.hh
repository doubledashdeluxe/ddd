#pragma once

#include "game/GameApp.hh"

#include <payload/Replace.hh>

class AwardApp : public GameApp {
public:
    REPLACE AwardApp();
    ~AwardApp() override;
    void draw() override;
    void calc() override;

    static AwardApp *Instance();

private:
    u32 m_state;

    static AwardApp *s_instance;
};
size_assert(AwardApp, 0x10);
