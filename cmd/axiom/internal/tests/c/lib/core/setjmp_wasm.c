/* setjmp is not available on WebAssembly targets (C3300). */
/* wasm-only */
/* expect-error: C3300 */
#include <setjmp.h>

static jmp_buf jb;

int main(void) {
    if (setjmp(jb))
        return 1;
    return 0;
}
