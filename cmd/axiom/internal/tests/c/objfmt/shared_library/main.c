// A shared library of this compiler (--emit=shared): a DLL, loaded with the
// program by its import directory, its constructor run by the loader, its
// thread-local and data; or a macOS dynamic library (its exports, install
// name, initializers), which the program is bound to.
// ours-flags: --c-std=gnu17
// expect: 10 103 one 10 7
// expect-dylib: @rpath/liboursso.dylib
#include <stdio.h>

int so_add(int a, int b);
int so_tls(int d);
const char *so_name(int i);
int so_hook(int x);
extern int counter;

int main(void) {
    int a = so_add(1, 2);
    int t = so_tls(3);
    printf("%d %d %s %d %d\n", a, t, so_name(1), so_hook(5), counter);
    return 0;
}
