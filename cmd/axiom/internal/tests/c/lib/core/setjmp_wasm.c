/* setjmp on WebAssembly without the exception-handling extension (not in
   the default features), which it needs to return to a frame: C3300,
   naming --features=+exception-handling. */
/* wasm-only */
/* expect-error: C3300 */
#include <setjmp.h>

static jmp_buf jb;

int main(void) {
    if (setjmp(jb))
        return 1;
    return 0;
}
