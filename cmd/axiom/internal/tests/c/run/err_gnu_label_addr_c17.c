/* flags: --c-std=c17 */
/* expect-error: C2001 */
int main(void) { void *p = &&out; out: return p == 0; }
