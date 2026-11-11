/* flags: --c-std=c11 */
/* expect-error: C3220 */
_Noreturn int main(void) { for (;;) {} }
