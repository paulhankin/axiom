/* expect-error: C3070 */
struct S { double d : 3; };
int main(void) { return 0; }
