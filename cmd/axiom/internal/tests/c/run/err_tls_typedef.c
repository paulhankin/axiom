/* flags: --c-std=c11 */
/* expect-error: C3076 */
typedef _Thread_local int T;
int main(void) { return 0; }
