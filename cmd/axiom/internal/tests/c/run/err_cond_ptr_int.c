/* expect-error: C3020 */
/* 1 is not a null pointer constant */
int main(void) { int c = 1; int *p = 0; return (c ? p : 1) == 0; }
