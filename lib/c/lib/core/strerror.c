/* strerror (<string.h>, C23 7.26.6.3): the message of error number errnum
   (the values of <errno.h>), or "Unknown error" for others. The strings are
   constant: a program must not modify them, and no call changes another's. */
#include <errno.h>
#include "axc/string_hosted.h"

static const char *message(int e) {
    switch (e) {
    case 0:
        return "No error";
    case 1:
        return "Operation not permitted";
    case 2:
        return "No such file or directory";
    case 3:
        return "No such process";
    case 4:
        return "Interrupted";
    case 5:
        return "Input/output error";
    case 6:
        return "No such device or address";
    case 7:
        return "Argument list too long";
    case 8:
        return "Not an executable";
    case 9:
        return "Bad file descriptor";
    case 10:
        return "No child process";
    case 11:
        return "Resource temporarily unavailable";
    case 12:
        return "Out of memory";
    case 13:
        return "Permission denied";
    case 14:
        return "Bad address";
    case 16:
        return "Resource busy";
    case 17:
        return "File exists";
    case 18:
        return "Cross-device link";
    case 19:
        return "No such device";
    case 20:
        return "Not a directory";
    case 21:
        return "Is a directory";
    case 22:
        return "Invalid argument";
    case 23:
        return "Too many open files in the system";
    case 24:
        return "Too many open files";
    case 25:
        return "Not a terminal";
    case 26:
        return "Text file busy";
    case 27:
        return "File too large";
    case 28:
        return "No space left on device";
    case 29:
        return "Not seekable";
    case 30:
        return "Read-only file system";
    case 31:
        return "Too many links";
    case 32:
        return "Broken pipe";
    case 33:
        return "Argument outside the domain";
    case 34:
        return "Result out of range";
    case 35:
        return "Deadlock avoided";
    case 36:
        return "File name too long";
    case 37:
        return "No locks available";
    case 38:
        return "Not implemented";
    case 39:
        return "Directory not empty";
    case 40:
        return "Too many symbolic links";
    case 75:
        return "Value too large for its type";
    case 84:
        return "Invalid or incomplete multibyte or wide character";
    case 95:
        return "Not supported";
    case 110:
        return "Timed out";
    default:
        return "Unknown error";
    }
}

char *strerror(int errnum) {
    return (char *)message(errnum);
}
