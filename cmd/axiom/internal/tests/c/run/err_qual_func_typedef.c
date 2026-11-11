/* flags: --c-std=c99 */
/* expect-error: C3006 */
/* 6.7.3p8: a function type with type qualifiers (through a typedef name). */
typedef int F(void);
const F f;
int main(void) { return 0; }
