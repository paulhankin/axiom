/* limits_inc.h - included by itself to LIMITS_INC_MAX levels (with the file
   that first includes it) by the translation limits tests (limits_c89.c,
   limits_c99.c). Written by cmd/axiom/internal/tests/tools/gen_c_limits.py. */
#if LIMITS_INC_DEPTH < LIMITS_INC_MAX
#if LIMITS_INC_DEPTH == 1
#undef LIMITS_INC_DEPTH
#define LIMITS_INC_DEPTH 2
#elif LIMITS_INC_DEPTH == 2
#undef LIMITS_INC_DEPTH
#define LIMITS_INC_DEPTH 3
#elif LIMITS_INC_DEPTH == 3
#undef LIMITS_INC_DEPTH
#define LIMITS_INC_DEPTH 4
#elif LIMITS_INC_DEPTH == 4
#undef LIMITS_INC_DEPTH
#define LIMITS_INC_DEPTH 5
#elif LIMITS_INC_DEPTH == 5
#undef LIMITS_INC_DEPTH
#define LIMITS_INC_DEPTH 6
#elif LIMITS_INC_DEPTH == 6
#undef LIMITS_INC_DEPTH
#define LIMITS_INC_DEPTH 7
#elif LIMITS_INC_DEPTH == 7
#undef LIMITS_INC_DEPTH
#define LIMITS_INC_DEPTH 8
#elif LIMITS_INC_DEPTH == 8
#undef LIMITS_INC_DEPTH
#define LIMITS_INC_DEPTH 9
#elif LIMITS_INC_DEPTH == 9
#undef LIMITS_INC_DEPTH
#define LIMITS_INC_DEPTH 10
#elif LIMITS_INC_DEPTH == 10
#undef LIMITS_INC_DEPTH
#define LIMITS_INC_DEPTH 11
#elif LIMITS_INC_DEPTH == 11
#undef LIMITS_INC_DEPTH
#define LIMITS_INC_DEPTH 12
#elif LIMITS_INC_DEPTH == 12
#undef LIMITS_INC_DEPTH
#define LIMITS_INC_DEPTH 13
#elif LIMITS_INC_DEPTH == 13
#undef LIMITS_INC_DEPTH
#define LIMITS_INC_DEPTH 14
#elif LIMITS_INC_DEPTH == 14
#undef LIMITS_INC_DEPTH
#define LIMITS_INC_DEPTH 15
#endif
#include "limits_inc.h"
#else
static int limits_inc_deepest = LIMITS_INC_DEPTH;
#endif
