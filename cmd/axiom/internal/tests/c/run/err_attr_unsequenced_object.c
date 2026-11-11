/* flags: --c-std=c23 */
/* expect-error: C3077 */
/* unsequenced and reproducible appertain to function types (6.7.13). */
[[unsequenced]] int x;
int main(void) { return 0; }
