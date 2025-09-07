#include "Color.hh"

bool operator==(const Color &a, const Color &b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

bool operator!=(const Color &a, const Color &b) {
    return !(a == b);
}
