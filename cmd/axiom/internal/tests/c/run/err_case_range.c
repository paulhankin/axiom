/* No case ranges (an extension). */
/* expect-error: C2001 */
int f(int x) { switch (x) { case 1 ... 3: return 1; } return 0; }
int main(void) { return f(0); }
