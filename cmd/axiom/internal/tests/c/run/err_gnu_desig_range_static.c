/* flags: --c-std=gnu17 */
/* expect-error: C3031 */
/* a designator range with effects in an object with static storage duration:
   not a constant (reported once) */
int f(void);
static int a[3] = { [0 ... 2] = f() };
int main(void) { return a[0]; }
