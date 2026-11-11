/* flags: --c-std=c17 */
/* expect-error: C3001 */
/* true is a keyword of C23: before, <stdbool.h> defines it. */
int main(void) { return true; }
