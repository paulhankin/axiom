/* flags: --c-std=c17 */
/* expect-error: C2100 */
int f(void) { return alignof(int); }
