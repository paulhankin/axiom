/* expect-error: C3900 */
int main(void) { int *p = (int[]){ 1, 2 }; return p[0]; }
