/* flags: --c-std=c89 */
/* expect-error: C1051 */
/* The _Pragma operator is C99's: before, `_Pragma (` is reported (the name
   alone is an identifier, reserved). */
int _Pragma;
_Pragma("once")
