/* A typedef of a variably modified type cannot be at file scope. */
/* expect-error: C3081 */
extern int n;
typedef int T[n];
int main(void) { return 0; }
