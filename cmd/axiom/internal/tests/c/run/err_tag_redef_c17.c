/* flags: --c-std=c17 */
/* expect-error: C3002 */
/* Before C23 a structure cannot be defined twice in a scope. */
struct S { int a; };
struct S { int a; };
int main(void) { return 0; }
