/* flags: --c-std=gnu11 */
/* The GNU dialects' syntax: attributes in their places, asm labels, statement
   expressions, local labels, labels as values, designator ranges, old
   designators, case ranges, `?:` without its middle operand, `goto *`,
   __alignof__ of an expression, __builtin_types_compatible_p. */
__extension__ typedef long long ll;
struct __attribute__((packed)) S { char c; int i __attribute__((aligned(2))); } __attribute__((aligned(4)));
static int f(int x __attribute__((unused))) __attribute__((noinline, cold));
int g(void) __asm__("gg") __attribute__((weak));
struct E {};
int main(void) {
    __label__ out;
    int y = ({ int z = 3; z + 1; });
    void *p = &&out;
    int a[10] = { [0 ... 4] = 1, [5] = 2 };
    struct { int q; } st = { q: 4 };
    switch (y) { case 1 ... 3: break; }
    y = y ?: 2;
    goto *p;
out: __attribute__((unused));
    return __alignof__ y + __builtin_types_compatible_p(int, long);
}
