/* flags: --c-std=c23 */
/* expect-error: C3230 */
int main(int argc, char **argv) { (void)argv; _Decimal128 x = 1; float f = 2; return (int)(argc ? x : f); }
