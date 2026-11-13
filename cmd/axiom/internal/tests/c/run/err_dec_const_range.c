/* flags: --c-std=c23 */
/* expect-error: C1018 */
int main(void) { _Decimal32 x = 1E97DF; return (int)x; }
