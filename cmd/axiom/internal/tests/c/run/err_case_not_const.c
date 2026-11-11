/* expect-error: C3035 */
int main(void) { int x = 1, y = 1; switch (x) { case y: return 1; } return 0; }
