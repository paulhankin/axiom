/* flags: --c-std=c99 */
/* expect-error: C3008 */
/* 6.7.5.3p3: the inner declarator of a definition is a declarator that is
   not the definition's, so its identifier list must be empty. */
int (*f(int x))(a) { return 0; }
int main(void) { return 0; }
