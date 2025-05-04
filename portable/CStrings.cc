#include "CStrings.hh"

extern "C" {
#include <ctype.h>
}

int CStrings::Strncasecmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        char c1 = tolower(s1[i]), c2 = tolower(s2[i]);

        if (c1 < c2) {
            return -1;
        }

        if (c1 > c2) {
            return 1;
        }

        if (c1 == '\0') {
            return 0;
        }
    }

    return 0;
}
