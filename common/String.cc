#include "String.hh"

extern "C" {
#include <string.h>
}

bool String::StartsWith(const char *string, const char *prefix) {
    size_t prefixLength = strlen(prefix);
    return !strncmp(string, prefix, prefixLength);
}

bool String::EndsWith(const char *string, const char *suffix) {
    size_t stringLength = strlen(string);
    size_t suffixLength = strlen(suffix);
    if (stringLength < suffixLength) {
        return false;
    }
    return !strcmp(string + stringLength - suffixLength, suffix);
}
