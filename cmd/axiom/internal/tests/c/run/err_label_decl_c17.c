/* A label before a declaration is a C23 feature. */
/* flags: --c-std=c17 */
/* expect-error: C2138 */
int main(void) { goto l; l: int x = 1; return x; }
