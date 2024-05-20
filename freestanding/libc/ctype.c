#include <ctype.h>

#ifdef __CWCC__
int isdigit(int c) {
    return c >= '0' && c <= '9';
}
#endif
