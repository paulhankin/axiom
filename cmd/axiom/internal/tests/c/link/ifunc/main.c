// Indirect functions (STT_GNU_IFUNC: the attribute ifunc, a GNU dialect)
// in a static program, whose start-up applies the IRELATIVE relocations:
// this compiler's and the system compiler's (a local one among them),
// called and taken the address of (one address wherever it is taken).
// ours-flags: --c-std=gnu17
// flags: --c-std=gnu17
// cc-variants: -O2 | -O0 -fno-pie
// expect: 10 12 103 1 1 104 1
int printf(const char *, ...);

static int impl_add(int x) { return x + 1; }
static int impl_mul(int x) { return x * 2; }
static int pick = 1;
static int (*resolve_twice(void))(int) { return pick ? impl_mul : impl_add; }
int twice(int) __attribute__((ifunc("resolve_twice")));
int (*twice_ptr)(int) = twice;  // (an address in data)

int sys_ifn(int);
int (*sys_ifn_addr(void))(int);
int sys_call_local(int);
int ours_twice(int);
int (*ours_twice_addr(void))(int);

int main(void) {
    int (*p)(int) = twice;
    printf("%d %d %d %d %d %d %d\n", twice(5), p(6), sys_ifn(3), sys_ifn_addr() == sys_ifn, p == twice_ptr,
           sys_call_local(5), ours_twice_addr() == twice && ours_twice(2) == 4);
    return 0;
}
