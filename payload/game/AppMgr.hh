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

    class Request {
    public:
        enum {
            DestroyApp = 1 << 0,
            RestartApp = 1 << 1,
            PrepareForMovieApp = 1 << 2,
            StartMovieApp = 1 << 3,
        };

    private:
        Request();
    };

    static s32 CurrentApp();
    static void Request(u32 requests);
    static void Draw();
    static void Calc();

private:
    AppMgr();

    static s32 s_currentApp; // msGameApp
    static u32 s_requests;
};
