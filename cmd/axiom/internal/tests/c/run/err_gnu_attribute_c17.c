/* flags: --c-std=c17 */
/* expect-error: C2001 */
/* GNU attributes are a GNU dialect's: not in C17. */
struct __attribute__((packed)) S { char c; int i; };
int main(void) { return 0; }
