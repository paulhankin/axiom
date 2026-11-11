/* expect-error: C3031 */
int main(void) { int y = 1; static int x = y; return x; }
