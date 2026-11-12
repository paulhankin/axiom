/* flags: --c-std=c11 */
/* expect-error: C3070 */
/* 6.7.2.1p5: atomic bit-fields are implementation-defined, and not supported. */
struct s { _Atomic unsigned x : 3; };
int main(void) { return 0; }
