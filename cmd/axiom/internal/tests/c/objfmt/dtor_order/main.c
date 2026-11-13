// The constructors and destructors of an object of this compiler, by
// priority: the constructors in increasing order of their priorities, the
// plain ones last; the destructors in the reverse order, the plain ones
// first, as the generic ABI has them (and as the C runtime runs the
// .CRT$XT* sections a COFF object names for them, forward). Run under Wine
// for x86_64-windows; built and checked for the others.
// ours-flags: --c-std=gnu17
// flags: --c-std=gnu17
// expect: ctor 150
// expect: ctor 200
// expect: ctor default
// expect: main 7
// expect: dtor default
// expect: dtor 200
// expect: dtor 150
#include <stdio.h>

int ours_d(void);

int main(void) {
    printf("main %d\n", ours_d());
    return 0;
}
