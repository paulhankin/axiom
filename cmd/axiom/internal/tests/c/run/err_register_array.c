/* expect-error: C3222 */
/* a register array cannot be converted to a pointer (its address) */
int main(void) { register int a[2] = { 1, 2 }; return a[1]; }
