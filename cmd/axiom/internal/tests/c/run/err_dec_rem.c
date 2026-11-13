/* flags: --c-std=c23 */
/* expect-error: C3020 */
int main(void) { _Decimal64 x = 7; return (int)(x % 2); }
