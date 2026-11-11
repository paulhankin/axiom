/* expect-error: C3044 */
int f(void) { return; }
int main(void) { return f(); }
