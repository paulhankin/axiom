// A dynamic library given by -l (its install name and exports), and one
// named only (bound by name, looked up when the program is loaded).
// targets: x86_64-macos aarch64-macos
// flags: -lz
// expect-dylib: @rpath/libdy.dylib
// expect-dylib: /usr/lib/libz.dylib
#include <stdio.h>

int dy_twice(int x);
extern int dy_value;
unsigned long compressBound(unsigned long n);
int tlv_next(void);

int main(void) {
    printf("%d %d %d %d\n", dy_twice(3), dy_value, compressBound(10) > 10, tlv_next());
    return 0;
}
