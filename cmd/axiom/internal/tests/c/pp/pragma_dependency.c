/* flags: --c-std=c11 */
/* expect-warning: C4112 */
/* #pragma GCC dependency: a file not changed after the current one says
   nothing; one that cannot be found is C4112. */
#pragma GCC dependency "pragma_dependency.c"
#pragma GCC dependency "no-such-file.h" something
int x;
