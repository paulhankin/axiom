/* No variably modified type at file scope. */
/* expect-error: C3081 */
extern int n;
int (*p)[n];
int main(void) { return 0; }
