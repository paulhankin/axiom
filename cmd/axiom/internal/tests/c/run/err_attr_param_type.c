/* flags: --c-std=c23 */
/* expect-error: C3077 */
int f(int [[deprecated]] p) { return p; }
int main(void) { return f(0); }
