/* _Decimal64x has no complex type (H.2.6: only the binary types do). */
/* flags: --c-std=c23 */
/* expect-error: C3005 */
_Decimal64x _Complex z;
int main(void) { return 0; }
