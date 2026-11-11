/* expect-error: C3222 */
int main(void) { register int r = 1; int *p = &r; return *p; }
