/* A case label is an integer constant expression. */
/* expect-error: C3035 */
int f(int x) { switch (x) { case 1.0: return 1; } return 0; }
int main(void) { return f(0); }
