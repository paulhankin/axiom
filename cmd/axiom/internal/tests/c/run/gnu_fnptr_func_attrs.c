/* flags: --c-std=gnu17 --c-warnings=error */
/* The GNU attributes of functions on what points to a function (a member,
   an object, a typedef, a parameter), as libraries' headers put format on
   their tables of functions: taken without a diagnostic; on another object
   they stay C4052. */
#include "host.h"
typedef int (*logger)(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
struct api {
    int (*printf_like)(const char *fmt, ...) __attribute__((__format__(__printf__, 1, 2)));
    void (*die)(const char *msg) __attribute__((noreturn));
    int count;
};
static int quiet(const char *fmt, ...) { return fmt[0]; }
static void (*handler)(const char *, ...) __attribute__((format(printf, 1, 2), nonnull(1)));
int main(void) {
    struct api a = { quiet, 0, 3 };
    logger l = quiet;
    print_i64(a.printf_like("A") + l("B") + a.count + (handler == 0));
    return 0;
}
/* expect: 135 */
