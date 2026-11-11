/* flags: --c-std=c99 */
/* expect-error: C2100 */
unsigned long n = _Alignof(int);
