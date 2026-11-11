/* main with its arguments: argv[argc] is a null pointer, and each of the
   strings before it is terminated (the host may give no program name). */
#include "host.h"

static void say(int n) { print_i64(n); }
static void none(void) { }

int main(int argc, char **argv) {
    int i, ok = 1;
    for (i = 0; i < argc; i++) {
        int n = 0;
        while (argv[i][n] && n < 100000) n++;
        if (n == 100000) ok = 0;
    }
    print_i64(argc >= 0);
    print_i64(argv[argc] == 0);
    print_i64(ok);
    argc >= 0 ? say(1) : none();
    argc < 0 ? say(2) : none();
    return 0;
}

/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
