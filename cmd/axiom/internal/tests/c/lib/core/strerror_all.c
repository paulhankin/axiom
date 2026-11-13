/* strerror (C17 7.24.6.2) has a message of its own for every code of
   <errno.h>, the library's (those any of its functions sets), and
   "Unknown error" for another number; the messages are distinct. */
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    static const struct {
        const char *name;
        int code;
    } codes[] = {
        {"EDOM", EDOM},
        {"ERANGE", ERANGE},
        {"EILSEQ", EILSEQ},
        {"EPERM", EPERM},
        {"ENOENT", ENOENT},
        {"ESRCH", ESRCH},
        {"EINTR", EINTR},
        {"EIO", EIO},
        {"ENXIO", ENXIO},
        {"E2BIG", E2BIG},
        {"ENOEXEC", ENOEXEC},
        {"EBADF", EBADF},
        {"ECHILD", ECHILD},
        {"EAGAIN", EAGAIN},
        {"ENOMEM", ENOMEM},
        {"EACCES", EACCES},
        {"EFAULT", EFAULT},
        {"EBUSY", EBUSY},
        {"EEXIST", EEXIST},
        {"EXDEV", EXDEV},
        {"ENODEV", ENODEV},
        {"ENOTDIR", ENOTDIR},
        {"EISDIR", EISDIR},
        {"EINVAL", EINVAL},
        {"ENFILE", ENFILE},
        {"EMFILE", EMFILE},
        {"ENOTTY", ENOTTY},
        {"ETXTBSY", ETXTBSY},
        {"EFBIG", EFBIG},
        {"ENOSPC", ENOSPC},
        {"ESPIPE", ESPIPE},
        {"EROFS", EROFS},
        {"EMLINK", EMLINK},
        {"EPIPE", EPIPE},
        {"EDEADLK", EDEADLK},
        {"ENAMETOOLONG", ENAMETOOLONG},
        {"ENOLCK", ENOLCK},
        {"ENOSYS", ENOSYS},
        {"ENOTEMPTY", ENOTEMPTY},
        {"ELOOP", ELOOP},
        {"EOVERFLOW", EOVERFLOW},
        {"ENOTSUP", ENOTSUP},
        {"ETIMEDOUT", ETIMEDOUT},
    };
    size_t i, j, n = sizeof codes / sizeof codes[0], bad = 0;
    for (i = 0; i < n; i++) {
        const char *m = strerror(codes[i].code);
        if (m == NULL || *m == 0 || strcmp(m, "Unknown error") == 0) {
            printf("%s: no message\n", codes[i].name);
            bad++;
        }
        for (j = 0; j < i; j++)
            if (strcmp(m, strerror(codes[j].code)) == 0) {
                printf("%s and %s: the same message\n", codes[i].name, codes[j].name);
                bad++;
            }
    }
    printf("%d codes, %d problems; EWOULDBLOCK is EAGAIN: %d\n", (int)n, (int)bad, EWOULDBLOCK == EAGAIN);
    printf("%s; %s\n", strerror(0), strerror(-7));
    return 0;
}

// expect: 43 codes, 0 problems; EWOULDBLOCK is EAGAIN: 1
// expect: No error; Unknown error
