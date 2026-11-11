/* expect-error: C3028 */
int main(void) { int *p = 0; double d = (double)p; return 0; }
