/* A declaration in for is a C99 feature. */
/* flags: --c-std=c95 */
/* expect-error: C2107 */
int main(void) { int s = 0; for (int i = 0; i < 3; i++) s += i; return s; }
