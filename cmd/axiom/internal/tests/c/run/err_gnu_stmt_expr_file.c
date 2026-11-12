/* flags: --c-std=gnu11 */
/* expect-error: C3090 */
int x = ({ 3; });
int main(void) { return 0; }
