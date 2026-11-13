/* The C library's files on a WebAssembly host whose file system is in
   memory (the node host's --files=memory: std/internal/runtime/memfs.mjs):
   fopen and its modes, positions, rename, remove, tmpfile, and directories'
   errors, all within the program. */
/* wasm-only */
/* host-flags: --files=memory */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "print.h"

int main(void) {
    char buf[64] = { 0 };
    FILE *f = fopen("a.txt", "w");
    FILE *t;
    print_bool(f != NULL); /* expect: true */
    fputs("hello, memory\n", f);
    fclose(f);
    f = fopen("/a.txt", "r"); /* (the working directory is the root) */
    print_bool(f != NULL && fgets(buf, sizeof buf, f) != NULL); /* expect: true */
    print_cstr(buf); /* expect: hello, memory */ /* expect: */
    fseek(f, 7, SEEK_SET);
    print_i64(fgetc(f)); /* expect: 109 */
    fclose(f);
    f = fopen("a.txt", "a+");
    fputs("more\n", f);
    rewind(f);
    print_i64((long)fread(buf, 1, sizeof buf, f)); /* expect: 19 */
    fclose(f);
    print_i64(rename("a.txt", "b.txt")); /* expect: 0 */
    print_bool(fopen("a.txt", "r") == NULL && errno == ENOENT); /* expect: true */
    print_bool(fopen("/tmp", "w") == NULL); /* expect: true */
    print_bool(fopen("/no/such/dir/x", "w") == NULL); /* expect: true */
    t = tmpfile();
    print_bool(t != NULL); /* expect: true */
    fputs("scratch", t);
    rewind(t);
    memset(buf, 0, sizeof buf);
    fgets(buf, sizeof buf, t);
    print_cstr(buf); /* expect: scratch */
    fclose(t);
    print_i64(remove("b.txt")); /* expect: 0 */
    print_bool(remove("b.txt") != 0); /* expect: true */
    return 0;
}
