/* #pragma lines and _Pragma operators are dropped, the STDC ones too. */
/* flags: --c-std=c99 */
a
#pragma once
#pragma STDC FP_CONTRACT ON
#pragma anything at all (
b _Pragma("message(\"x\")") c
#define P(x) _Pragma(#x) d
P(omp parallel) e
_Pragma(L"wide")
f _Pragma
("split")
# /* the null directive */
#
g
