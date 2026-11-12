/* flags: --c-std=gnu11 */
/* expect-error: C3090 */
int f(void) __attribute__((alias("nowhere")));
int main(void) { return f(); }
