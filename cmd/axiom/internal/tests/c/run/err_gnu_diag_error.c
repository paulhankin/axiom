/* flags: --c-std=gnu11 */
/* expect-error: C4050 */
/* #pragma GCC diagnostic error makes the warning an error (its code kept). */
int old(void) __attribute__((deprecated));
#pragma GCC diagnostic error "-Wdeprecated-declarations"
int main(void) { return old(); }
int old(void) { return 0; }
