/* flags: --c-std=c17 */
/* expect-error: C1019 */
int main(void) { double x = 1.5dd; return (int)x; }
