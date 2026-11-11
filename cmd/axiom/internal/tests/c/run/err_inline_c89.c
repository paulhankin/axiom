/* flags: --c-std=c89 */
/* expect-error: C2100 */
inline int f(void) { return 0; }
int main(void) { return f(); }
