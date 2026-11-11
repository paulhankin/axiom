/* flags: --c-std=c89 */
/* expect-error: C3006 */
const const int x = 1;
int main(void) { return 0; }
