/* flags: --c-std=c11 */
/* _Alignas, _Alignof, _Noreturn, _Thread_local (C11). */
_Alignas(16) char buf[16];
_Alignas(int) char buf2[4];
_Thread_local int tl;
static _Thread_local int tl2;
_Noreturn void quit(void);
unsigned long al = _Alignof(double);
