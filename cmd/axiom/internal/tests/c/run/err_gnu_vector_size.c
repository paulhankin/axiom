/* flags: --c-std=gnu11 */
/* expect-error: C3901 */
typedef int v4si __attribute__((vector_size(16)));
int main(void) { return 0; }
