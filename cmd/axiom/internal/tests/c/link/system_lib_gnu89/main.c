/* --c-lib=system in gnu89 (a cc's -std=gnu89, -std=c89 and -ansi, which
   build systems ask for C90 programs): the system's headers use long long,
   the dialect's extension there, and the program's GNU89 code is as it
   was (declarations after statements, inline's GNU89 meaning). */
/* flags: --c-lib=system --c-std=gnu89 --c-gnu-compat */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

inline int twice(int x) { return x * 2; }

int main(void) {
    char buf[32];
    long long big = 1LL << 40;
    struct stat st;
    sprintf(buf, "%lld", big);
    puts(buf); /* expect: 1099511627776 */
    printf("%d %d\n", stat(".", &st) == 0, twice(21)); /* expect: 1 42 */
    return 0;
}
