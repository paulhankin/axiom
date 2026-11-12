/* flags: --c-std=c17 --target=aarch64-macos */
/* check-only */
/* <stdatomic.h> agrees with <stdint.h> and <stddef.h> for the target (hdr_atomic_checks.h). */
#include "hdr_checks.h"
int main(void) { return hc_run() - 42; }
