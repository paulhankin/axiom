/* flags: --c-std=c23 */
/* expect-error: C3002 */
/* A structure defined again in the same scope must have the same members (C23 6.7.2.3). */
struct S { int a; };
struct S { long a; };
int main(void) { return 0; }
