/* flags: --c-std=c23 */
/* expect-error: C3077 */
/* Attributes after the declaration specifiers appertain to the type: deprecated does not apply to types. */
int [[deprecated]] x;
int main(void) { return x; }
