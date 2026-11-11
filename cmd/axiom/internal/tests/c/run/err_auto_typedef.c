/* flags: --c-std=c23 */
/* expect-error: C3006 */
typedef auto T = 1;
int main(void) { return 0; }
