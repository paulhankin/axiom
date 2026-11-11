/* flags: --c-std=c23 */
/* expect-error: C3078 */
int v = (register int){ 1 };
int main(void) { return 0; }
