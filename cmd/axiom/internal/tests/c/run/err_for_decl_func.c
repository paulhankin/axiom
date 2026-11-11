/* flags: --c-std=c99 */
/* expect-error: C3006 */
/* 6.8.5p3: a for statement's declaration declares only objects. */
int main(void) { for (int f(void); ; ) break; return 0; }
