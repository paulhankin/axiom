// Indirect functions in a dynamically linked program (ifunc/: a static
// one), whose IRELATIVE relocations the dynamic linker applies, and in a
// shared library of this compiler.
// ours-flags: --c-std=gnu17
// flags: --c-std=gnu17 --c-lib=system
// cc-variants: -O2 | -O0 -fno-pie
// expect: 10 12 103 1 1 104 1
// expect: 22 1
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
int so_ifn(int);
int so_calls(int);
int (*so_ifn_addr(void))(int);

int main(void) {
    int (*p)(int) = twice;
    printf("%d %d %d %d %d %d %d\n", twice(5), p(6), sys_ifn(3), sys_ifn_addr() == sys_ifn, p == twice_ptr,
           sys_call_local(5), ours_twice_addr() == twice && ours_twice(2) == 4);
    printf("%d %d\n", so_calls(3), so_ifn_addr() == so_ifn && so_ifn(1) == 8);
    return 0;
}
