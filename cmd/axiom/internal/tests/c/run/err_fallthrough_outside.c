/* C23: a fallthrough declaration must be in a switch statement. */
/* flags: --c-std=c23 */
/* expect-error: C3077 */
int main(void) { int x = 0; [[fallthrough]]; return x; }
