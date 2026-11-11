/* flags: --c-std=c23 */
/* expect-error: C3028 */
/* nullptr_t converts only to void, bool and pointer types */
int main(void) { return (int)nullptr; }
