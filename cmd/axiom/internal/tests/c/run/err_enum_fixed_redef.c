/* flags: --c-std=c23 */
/* expect-error: C3002 */
enum E : int { A };
enum E : int { B };
int main(void) { return 0; }
