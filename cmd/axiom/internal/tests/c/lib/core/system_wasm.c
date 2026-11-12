/* system (C17 7.22.4.8) under WebAssembly: no command processor. */
/* wasm-only */
#include <stdlib.h>
#include "print.h"

int main(void) {
    print_i64(system(NULL)); /* expect: 0 */
    print_i64(system("exit 3")); /* expect: -1 */
    return 0;
}
