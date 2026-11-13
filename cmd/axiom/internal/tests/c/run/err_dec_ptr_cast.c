/* flags: --c-std=c23 */
/* expect-error: C3028 */
int main(void) { _Decimal64 x = 7; void *p = (void *)x; return p == 0; }
