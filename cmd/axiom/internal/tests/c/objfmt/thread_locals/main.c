// Thread-locals as macOS has them (descriptors in __thread_vars:
// --macos-tls=platform, which --emit=obj implies), across toolchains: the program's own, a platform object's (sys_tl.c, which reads
// ours too), this compiler's object's (ours_tl.c, --emit=obj), a dynamic
// library's (dylib_tl.c, linked by the system's linker: bound by dyld), all
// through their descriptors; and cc_main.c, the platform's compiler's main,
// linked by the system's linker with this compiler's object.
// targets: x86_64-macos aarch64-macos
// flags: --macos-tls=platform
// ours-flags: --c-std=gnu17
// expect: main 1 sys 7 ours 35 dy 3 both 42 next 36
// expect-dylib: @rpath/libdy.dylib
#include <stdio.h>

_Thread_local int main_tl = 1;
extern _Thread_local int sys_tl;
extern _Thread_local long ours_tl;
extern _Thread_local int dy_tl;
long sys_sum(void);
long ours_next(void);

int main(void) {
    printf("main %d sys %d ours %ld dy %d both %ld next %ld\n", main_tl, sys_tl, ours_tl, dy_tl, sys_sum(),
           ours_next());
    return 0;
}
