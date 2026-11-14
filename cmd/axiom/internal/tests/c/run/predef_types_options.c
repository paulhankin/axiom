/* The conventional macros of the types follow --c-int, --c-long and
   --c-char (int of 16 bits, long of 32: no 64-bit long, and plain char
   unsigned) in a freestanding GNU dialect, which has __int128. */
/* check-only */
/* flags: --target=x86_64 --c-std=gnu23 --c-int=16 --c-long=32 --c-char=unsigned --c-freestanding */
#include "predef_checks.h"
