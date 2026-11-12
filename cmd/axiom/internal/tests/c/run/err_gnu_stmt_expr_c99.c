/* flags: --c-std=c99 */
/* expect-error: C2001 */
int main(void) { int x = ({ 1; }); return x; }
