/* flags: --c-std=c23 */
/* expect-error: C3020 */
int main(void) { return nullptr + 1 != 0; }
