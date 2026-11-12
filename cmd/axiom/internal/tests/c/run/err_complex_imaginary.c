/* The imaginary types are not supported (C3901). */
/* expect-error: C3901 */
int main(void) { _Imaginary double z; return 0; }
