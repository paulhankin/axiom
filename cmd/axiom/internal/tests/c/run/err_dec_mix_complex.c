/* flags: --c-std=c23 */
/* expect-error: C3230 */
int main(void) { _Decimal64 x = 1; double _Complex z = 2; return x == z; }
