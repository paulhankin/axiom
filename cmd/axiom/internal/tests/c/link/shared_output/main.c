// A shared library of this compiler (--emit=shared): position-independent
// code, its exports, the initial-exec thread-locals, addresses relocated
// where it is loaded, its constructor run by the dynamic linker; linked
// with a program of this compiler and loaded by the system toolchain's
// (dlopen).
// flags: --c-lib=system
// ours-flags: --c-std=gnu17
// expect: 10 105 one 7
// expect: so says 7 2
// dl-expect: 10 101 103 two 13 7
// dl-expect: so says 7 4
int printf(const char *, ...);

int so_add(int, int);
int so_tls(int);
const char *so_name(int);
extern int counter;
void so_print(void);

int main(void) {
    int a = so_add(1, 2);
    int t = so_tls(5);
    printf("%d %d %s %d\n", a, t, so_name(1), counter);
    so_print();
    return 0;
}
