/* flags: --c-std=c11 --c-int=16 --c-long=32 */
/* The headers' macros and types agree (hdr_checks.h), with int of 16 bits and long of 32, in c11. */
#include "hdr_checks.h"
#include "host.h"

int main(void) {
    print_i64(hc_run());
    return 0;
}

/* expect: 42 */
