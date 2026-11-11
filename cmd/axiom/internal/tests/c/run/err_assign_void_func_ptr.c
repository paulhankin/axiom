/* expect-error: C3022 */
/* void * converts to and from pointers to objects, not to functions */
static void f(void) {}
int main(void) { void *v = f; return v != 0; }
