/* flags: --c-std=c11 */
/* _Atomic followed by '(' is the type specifier; otherwise a qualifier. */
_Atomic(int) a;
_Atomic int b;
_Atomic const int *c;
int *_Atomic d;
_Atomic(int *) e;
