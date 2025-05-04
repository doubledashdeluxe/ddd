extern "C" {
#include <ctype.h>
}

#include <portable/CCtype.hh>

int isdigit(int c) {
    return CCtype::Isdigit(c);
}

int tolower(int c) {
    return CCtype::Tolower(c);
}
