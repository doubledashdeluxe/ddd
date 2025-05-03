#pragma once

#include <portable/Logger.hh>

class CubeLogger : public Logger {
public:
    void vlog(u32 level, const char *shortFormat, const char *longFormat, va_list vlist) override;

    static void Init();

private:
    CubeLogger();

    static CubeLogger *s_instance;
};
