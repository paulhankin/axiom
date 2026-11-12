/* flags: --c-std=c11 */
/* expect-error: C3020 */
int main(void) { char b[4]; void *p = b; p = p + 1; return 0; }
