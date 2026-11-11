/* expect-error: C3070 */
struct S { _Bool b : 2; };
int main(void) { return 0; }
