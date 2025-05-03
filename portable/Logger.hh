#pragma once

#include <portable/Log.hh>
#include <portable/Types.hh>

extern "C" {
#include <stdarg.h>
}

class Logger {
public:
    virtual void vlog(u32 level, const char *shortFormat, const char *longFormat, va_list vlist) = 0;

    static Logger *Instance();

protected:
    Logger();
    ~Logger();

private:
    static Logger *s_instance;
};
