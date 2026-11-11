/* expect-error: C3020 */
/* relational operators take pointers to object types only */
static void f(void) {}
static void g(void) {}
int main(void) { return f < g; }
