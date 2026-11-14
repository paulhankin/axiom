/* flags: --c-std=gnu17 */
/* expect-error: C3022 */
/* A GNU dialect's '?:' of a void and a non-void operand is void: its value
   cannot initialize an object. */
static void f(void) { }
int main(void) {
    int i = 1;
    int x = i ? i : f();
    return x;
}
