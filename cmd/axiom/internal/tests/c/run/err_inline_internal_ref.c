/* flags: --c-std=c11 */
/* expect-error: C3221 */
/* an inline definition refers to an identifier with internal linkage */
static int hidden = 3;
inline int peek(void) { return hidden; }
int main(void) { return 0; }
