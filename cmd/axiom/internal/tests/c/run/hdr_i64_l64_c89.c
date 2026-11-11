/* flags: --c-std=c89 --c-int=64 --c-long=64 */
/* The headers' macros and types agree (hdr_checks.h), with int of 64 bits and long of 64, in c89. */
#include "hdr_checks.h"
#include "host.h"

int main(void) {
    print_i64(hc_run());
    return 0;
}

/* expect: 42 */
