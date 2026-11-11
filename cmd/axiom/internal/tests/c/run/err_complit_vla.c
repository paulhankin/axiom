/* flags: --c-std=c99 */
/* expect-error: C3078 */
/* 6.5.2.5p1: a compound literal's type is not a variable length array. */
int main(void) { int n = 2; int *p = (int[n]){ 1, 2 }; return p[0]; }
