/* flags: --c-std=gnu11 */
/* expect-error: C3901 */
/* A packed bit-field that no unit within its structure holds. */
struct __attribute__((packed)) S { int b : 20; char c : 4; };
int main(void) { struct S s; s.b = 1; return s.b; }
