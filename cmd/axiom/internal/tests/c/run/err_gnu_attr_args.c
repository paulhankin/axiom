/* flags: --c-std=gnu11 */
/* expect-error: C3071 */
/* expect-error: C3077 */
int a __attribute__((aligned(3)));
int b __attribute__((section(1)));
int c __attribute__((visibility("public")));
int main(void) { return 0; }
