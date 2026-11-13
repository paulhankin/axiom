#include <errno.h>
#include <unistd.h>
extern int so_counter;
extern int so_table[3];
extern __thread int so_tls;
int so_get(void);
int so_tls_get(void);
int sys_posix(void) {
    errno = 0;
    int r = close(-1);
    return r == -1 && errno == EBADF;
}
int sys_lib(void) {
    so_counter += 100;
    int a = so_get();
    so_tls = 9;
    return a * 100 + so_table[2] * 10 + so_tls_get();
}
