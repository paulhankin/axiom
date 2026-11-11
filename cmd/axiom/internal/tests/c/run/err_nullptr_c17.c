/* flags: --c-std=c17 */
/* expect-error: C3001 */
/* nullptr is a keyword of C23: before, an identifier like any other. */
int main(void) { void *p = nullptr; return p != 0; }
