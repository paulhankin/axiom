/* flags: --c-std=gnu17 */
/* native-only */
/* expect-error: C3043 */
/* asm goto's labels are the function's (here one that is not defined). */
int f(int x) {
    asm goto("" :::: missing);
    return x;
}
int main(void) { return 0; }
