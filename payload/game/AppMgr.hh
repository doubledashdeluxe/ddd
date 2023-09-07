#pragma once

#include <common/Types.hh>

class AppMgr {
public:
    class KartAppEnum {
    public:
        enum {
            Title = 10,
        };

    private:
        KartAppEnum();
    };

    static s32 CurrentApp();

private:
    AppMgr();

    static s32 s_currentApp; // msGameApp
};
