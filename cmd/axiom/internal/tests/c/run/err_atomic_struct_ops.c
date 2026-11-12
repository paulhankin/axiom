/* flags: --c-std=c11 */
/* expect-error: C3020 */
/* an atomic structure is no scalar */
struct s { int a; };
_Atomic struct s x;
int main(void) { x++; return 0; }
