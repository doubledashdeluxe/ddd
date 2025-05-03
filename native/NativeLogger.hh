#pragma once

#include <portable/Logger.hh>

class NativeLogger : public Logger {
public:
    void vlog(u32 level, const char *shortFormat, const char *longFormat, va_list vlist) override;

private:
    NativeLogger();
    ~NativeLogger();

    static NativeLogger s_instance;
};
