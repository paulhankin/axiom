/* flags: --c-std=c99 */
/* expect-error: C3010 */
/* implicit int for an undeclared parameter is not in C99 */
int f(a) { return a; }
int main(void) { return f(1); }
