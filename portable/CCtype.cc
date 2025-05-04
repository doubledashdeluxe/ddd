#include "CCtype.hh"

int CCtype::Isdigit(int c) {
    return c >= '0' && c <= '9';
}

int CCtype::Tolower(int c) {
    if (c >= 'A' && c <= 'Z') {
        c += 'a' - 'A';
    }
    return c;
}
