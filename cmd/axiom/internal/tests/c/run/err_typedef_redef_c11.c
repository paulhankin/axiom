/* flags: --c-std=c11 */
/* expect-error: C3002 */
/* A typedef name defined again must name the same type. */
typedef int T;
typedef long T;
int main(void) { return 0; }
