/* flags: --c-std=c17 */
/* expect-error: C3060 */
/* The GNU builtins are a GNU dialect's: in C17 a call of an undeclared function. */
int main(void) { return __builtin_expect(1, 1); }
