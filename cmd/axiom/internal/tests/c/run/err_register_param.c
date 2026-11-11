/* expect-error: C3222 */
static int f(register int r) { return *&r; }
int main(void) { return f(1); }
