#include <ctype.h>

int isdigit(int c) {
    return c >= '0' && c <= '9';
}

int tolower(int c) {
    if (c >= 'A' && c <= 'Z') {
        c += 'a' - 'A';
    }
    return c;
}
