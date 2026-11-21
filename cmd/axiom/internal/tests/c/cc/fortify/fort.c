/* Calls of the functions with variable arguments that the system's headers
   wrap under _FORTIFY_SOURCE (sprintf, snprintf, printf, fprintf, dprintf,
   open, error), whose wrappers forward the variable arguments with
   __builtin_va_arg_pack: inlined, the checked forms of the library are
   called. With an argument, sprintf writes it into an array too small. */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>

static int total(int n, ...) {
    va_list ap;
    va_start(ap, n);
    int s = 0;
    for (int i = 0; i < n; i++) s += va_arg(ap, int);
    va_end(ap);
    return s;
}

int main(int argc, char **argv) {
    char buf[8];
    int n = sprintf(buf, "%d", 1234567);
    printf("%s %d\n", buf, n);
    n = snprintf(buf, sizeof buf, "%s-%d", "abcdefghijkl", total(3, 1, 2, 3));
    printf("%s %d\n", buf, n);
    fprintf(stdout, "%c%c%c %.2f %lld\n", 'x', 'y', 'z', 2.5, 1LL << 40);
    int flags = argc > 9 ? O_RDWR : O_RDONLY;
    int fd = open("/dev/null", flags);
    printf("open %d\n", fd >= 0);
    close(fd);
    fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600);
    dprintf(fd, "%s %d\n", "written", 42);
    close(fd);
    printf("%s\n", "done");
    fflush(stdout);
    if (argc > 1) {
        n = sprintf(buf, "%s", argv[1]);
        printf("wrote %d\n", n);
    }
    return 0;
}
