/* flags: --c-std=c89 */
/* expect-error: C2109 */
int f(void) { return (int){1}; }
