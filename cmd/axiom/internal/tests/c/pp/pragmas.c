/* flags: --c-std=c11 */
/* The pragmas, in every mode: once, push_macro and pop_macro, message,
   pack (its forms), weak, STDC (FENV_ACCESS ON is warned about: not
   honoured), GCC diagnostic, visibility, system_header (no effect in the
   main file), and _Pragma; unknown ones are ignored. */
/* expect-warning: C4110 */
/* expect-warning: C4113 */
/* expect-warning: C4112 */
/* expect-warning: C4114 */
#include "inc/once.h"
#include "inc/once.h"
#define M 1
#pragma push_macro("M")
#undef M
#define M 2
M
#pragma pop_macro("M")
M
#pragma push_macro("N")
#define N 3
#pragma pop_macro("N")
N
#pragma message("a message")
#pragma message "another"
#pragma pack(push, 4)
#pragma pack(push, outer, 2)
#pragma pack(show)
#pragma pack(pop, outer)
#pragma pack(pop)
#pragma pack(8)
#pragma pack()
#pragma pack(3)
#pragma weak w
#pragma weak w2 = target
#pragma STDC FP_CONTRACT OFF
#pragma STDC CX_LIMITED_RANGE ON
#pragma STDC FENV_ACCESS ON
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic pop
#pragma GCC visibility push(hidden)
#pragma GCC visibility pop
#pragma GCC system_header
#pragma GCC optimize("O2")
#pragma something unknown
_Pragma("message(\"from _Pragma\")") after
#define DO_PRAGMA(x) _Pragma(#x)
DO_PRAGMA(GCC diagnostic warning "-Wall") end
