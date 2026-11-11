/* Skipped groups: only conditional directives count; other lines need only be tokens. */
#if 0
#error not seen
#include "nope.h"
#define X 1
#unknown directive
don't stop at an apostrophe /* or a comment's start
#else
#endif
#if 0
# if 1 / 0
# elif 1 / 0
# else
# endif
#elif 1
ok1
#elif 1 / 0
#else
#endif
#ifdef X
bad
#endif
#if 0
"unterminated string
#endif
ok2
