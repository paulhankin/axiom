/* expect-error: C3078 */
void f(void) { (void (void)){ 0 }; }
int main(void) { return 0; }
