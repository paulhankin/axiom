/* flags: --c-std=gnu11 */
/* expect-error: C3082 */
struct big { int a[4]; } b;
int main(void) { int v = 0; __atomic_add_fetch(&v, 1, 0); __atomic_load_n(&b, 0); return 0; }
