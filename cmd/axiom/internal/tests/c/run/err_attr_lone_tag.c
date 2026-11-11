/* flags: --c-std=c23 */
/* expect-error: C3077 */
/* Attributes before a declaration without declarators (6.7p1: the syntax wants them after the keyword). */
[[deprecated]] struct S { int a; };
int main(void) { return 0; }
