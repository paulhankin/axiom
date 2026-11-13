/* Streams at quick_exit and _Exit: whether unwritten buffered data are
   written is implementation-defined (C17 7.22.4.5p2); this library does
   not write them, nor remove tmpfile's files (exit does both). Standard
   output is a file here, so fully buffered: the line printed before
   quick_exit is lost, the one fflush wrote is not. */
#include <stdio.h>
#include <stdlib.h>

static void handler(void) {
    fputs("from at_quick_exit, buffered too\n", stdout);
}

int main(void) {
    printf("written\n");
    fflush(stdout);
    printf("lost\n");
    at_quick_exit(handler);
    quick_exit(0);
}
// expect: written
