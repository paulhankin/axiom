/* flags: --c-std=c23 */
/* expect-error: C3077 */
int a[2] [[deprecated]];
int main(void) { return 0; }
