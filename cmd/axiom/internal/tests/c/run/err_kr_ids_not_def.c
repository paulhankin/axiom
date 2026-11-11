/* flags: --c-std=c89 */
/* expect-error: C3008 */
/* 6.7.5.3p3: an identifier list only in the declarator of a definition. */
int f(a, b);
int main(void) { return 0; }
