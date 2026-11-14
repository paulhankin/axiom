#include <math.h>
#include <float.h>
double second(void) { return -INFINITY + (isnan(NAN) ? 0.0 : 1.0); }
