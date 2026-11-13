/* Thread-local variables of an object of the system's C compiler, compiled
   for each of its TLS models: local-exec and initial-exec (position-
   dependent and -independent executables), general- and local-dynamic
   (-fPIC), descriptors (-mtls-dialect=gnu2 on x86-64, the default on
   AArch64) and AArch64's traditional general-dynamic: the linker relaxes
   them, the program's thread-locals and the object's are one block. */
/* cc-variants: -O2 | -O2 -fPIC | -O0 -fno-pie | -O2 -fno-pie */
/* cc-variants-x86_64: -O2 -fPIC -mtls-dialect=gnu2 */
/* cc-variants-aarch64: -O2 -fPIC -mtls-dialect=trad */
#include <stdio.h>

_Thread_local int ours = 100;
int tls_sum(void);
int tls_bump(int by);
int *tls_addr(void);

int main(void) {
    printf("%d\n", tls_sum()); /* expect: 106 */
    printf("%d\n", tls_bump(5)); /* expect: 75 */
    printf("%d\n", tls_bump(1)); /* expect: 86 */
    *tls_addr() = 40;
    printf("%d\n", tls_sum()); /* expect: 52 */
    printf("%d\n", ours); /* expect: 40 */
    return 0;
}
