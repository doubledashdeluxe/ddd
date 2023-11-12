#include "J2DPane.hh"

extern "C" {
#include <string.h>
}

J2DPane *J2DPane::search(const char *tag) {
    size_t length = strlen(tag);
    if (length > 8) {
        return nullptr;
    }

    u64 uTag = 0;
    memcpy(&uTag, tag, sizeof(uTag));
    uTag >>= (8 - length) * 8;
    return search(uTag);
}
