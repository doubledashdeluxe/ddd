#include "Log.hh"

#include "portable/Logger.hh"

void Log(u32 level, const char *shortFormat, const char *longFormat, ...) {
    va_list vlist;
    va_start(vlist, longFormat);
    VLog(level, shortFormat, longFormat, vlist);
    va_end(vlist);
}

void VLog(u32 level, const char *shortFormat, const char *longFormat, va_list vlist) {
    Logger *logger = Logger::Instance();
    if (logger) {
        logger->vlog(level, shortFormat, longFormat, vlist);
    }
}
