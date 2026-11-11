/* flags: --c-std=c89 */
/* expect-error: C3006 */
typedef const int T;
const T x = 1;
int main(void) { return 0; }
