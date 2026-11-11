/* flags: --c-std=c11 */
/* expect-error: C3029 */
/* _Alignof of an incomplete type (6.5.3.4p1): an array of unknown length too. */
int x = _Alignof(int[]);
int main(void) { return x; }
