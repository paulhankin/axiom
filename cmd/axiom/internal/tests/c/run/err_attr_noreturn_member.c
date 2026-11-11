/* flags: --c-std=c23 */
/* expect-error: C3077 */
struct S { [[noreturn]] int a; };
int main(void) { return 0; }
