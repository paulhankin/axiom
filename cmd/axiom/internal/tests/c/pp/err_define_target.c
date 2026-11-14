/* A macro of the target's may be redefined, but a definition of the
   program's then follows the usual rule (6.10.3p2). */
/* expect-error: C1060 */
#define __linux__ 2
#define __linux__ 3
