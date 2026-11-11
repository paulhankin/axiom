/* A macro invocation ends in the file it starts in. */
/* expect-error: C1063 */
#define unfinished(a, b) a b
#include "inc/open_call.h"
2)
