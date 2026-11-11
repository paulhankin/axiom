/* Declarations after statements are a C99 feature. */
/* flags: --c-std=c89 */
/* expect-error: C2106 */
int main(void) { int a = 1; a++; int b = a; return b; }
