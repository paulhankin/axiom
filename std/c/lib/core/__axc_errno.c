/* __axc_errno (<errno.h>): the address of errno's storage, which is each
   thread's own (C11 7.5p2: errno has thread storage duration). */
#include <errno.h>

static _Thread_local int value;

int *__axc_errno(void) {
    return &value;
}
