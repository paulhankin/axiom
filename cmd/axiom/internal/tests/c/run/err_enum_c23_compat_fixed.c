/* flags: --c-std=c23 */
/* expect-error: C3022 */
/* Enumerations with the same tag and constants but other fixed underlying
   types are not compatible (C23 6.2.7p1). */
enum E : long { A, B } ee;
int main(void) {
    enum E : int { A, B } e = A;
    enum E *p = &ee;
    return e + (p != 0);
}
