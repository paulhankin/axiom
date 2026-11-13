/* system (C17 7.22.4.8) on WebAssembly, which has no command processor:
   system(NULL) is 0, and a command is not run (-1). */
/* wasm-only */
#include <stdlib.h>
#include "print.h"

int main(void) {
    print_i64(system(NULL)); /* expect: 0 */
    print_i64(system("echo not run")); /* expect: -1 */
    return 0;
}
