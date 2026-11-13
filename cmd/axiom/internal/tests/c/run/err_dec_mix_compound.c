/* flags: --c-std=c23 */
/* expect-error: C3230 */
int main(void) { _Decimal32 x = 1; double d = 2; d += x; return 0; }
