#include "conf.h"
int a = FROM_INC;
int b = PRE;
#ifdef DEF
int c = DEF;
#endif
#ifdef GONE
int gone;
#endif
long v = __STDC_VERSION__;
#if defined(__x86_64__) || defined(__aarch64__)
int arch = 1;
#endif
