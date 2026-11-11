/* flags: --c-std=c89 */
/* expect-error: C3003 */
/* a prototype with a parameter that promotion changes is not compatible with () */
int f(float);
int f();
int main(void) { return 0; }
