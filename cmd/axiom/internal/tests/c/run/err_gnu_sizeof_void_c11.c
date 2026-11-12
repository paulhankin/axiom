/* flags: --c-std=c11 */
/* expect-error: C3029 */
int main(void) { return (int)sizeof(void); }
