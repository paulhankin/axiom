/* system (<stdlib.h>, C23 7.24.4.8): with a null pointer, whether there is
   a command processor; else the command's status from it (std/c/rt/core.ax:
   /bin/sh on Linux and macOS, POSIX's encoding of the status; cmd.exe on
   Windows; none under WebAssembly, -1). */
#include <stdlib.h>
#include "axc/core_impl.h"

int system(const char *string) {
    return (int)__axc_system(string);
}
