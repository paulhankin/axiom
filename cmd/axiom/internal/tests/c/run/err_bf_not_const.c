/* expect-error: C3035 */
int w = 3;
struct S { int a : w; };
int main(void) { return 0; }
