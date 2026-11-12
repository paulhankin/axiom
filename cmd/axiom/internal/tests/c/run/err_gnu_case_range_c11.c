/* flags: --c-std=c11 */
/* expect-error: C2001 */
int main(void) { int x = 2; switch (x) { case 1 ... 3: return 1; } return 0; }
