/* expect-error: C3078 */
struct S;
void f(void) { (struct S){ 0 }; }
int main(void) { return 0; }
