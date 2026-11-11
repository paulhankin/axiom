/* A label at the end of a block is a C23 feature. */
/* flags: --c-std=c11 */
/* expect-error: C2138 */
int main(void) { goto l; l: }
