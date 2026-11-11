/* flags: --c-std=c89 */
/* expect-error: C3070 */
struct S { char c : 3; };
int main(void) { return 0; }
