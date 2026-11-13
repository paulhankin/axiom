/* flags: --c-std=gnu17 */
/* expect-error: C2143 */
/* __declspec(thread) only among declaration specifiers */
struct __declspec(thread) S { int a; };
int main(void) { return 0; }
