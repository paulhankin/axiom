/* expect-error: C3035 */
/* a floating constant in an integer constant expression is only the immediate operand of a cast */
enum { A = (int)(1.5 + 1) };
int main(void) { return A; }
