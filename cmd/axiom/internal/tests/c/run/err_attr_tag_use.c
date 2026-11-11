/* flags: --c-std=c23 */
/* expect-error: C3077 */
/* Attributes in a struct specifier without a list appear only in `struct [[...]] S;` (6.7.2.3). */
struct S;
struct [[deprecated]] S *p;
int main(void) { return 0; }
