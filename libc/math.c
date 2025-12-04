#ifdef __CWCC__
#include <math.h>

double fabs(double x) {
    return __fabs(x);
}
#endif
