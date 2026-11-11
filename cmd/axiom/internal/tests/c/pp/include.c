/* "..." is looked for next to the file that includes, then in the -I directories. */
/* flags: -Iinc */
#include "inc/a.h"
#include "inc/guard.h"
#include "inc/guard.h"
#include <angle.h>
#include "d.h"
#define H "inc/guard.h"
#include H
#define A <angle.h>
#include A
#define LT <
#include LT angle.h>
#include "inc/nonl.h"
next
__LINE__ __FILE__
