extern "C" {
#include "OSError.h"
}

#include <cube/Log.hh>

extern "C" {
#include <stdio.h>
}

void OSReport(const char *format, ...) {
    va_list vlist;
    va_start(vlist, format);
    VDEBUG(format, vlist);
    va_end(vlist);
}
