/* fopen_s's files (C11 K.3.5.2.1p7): a file it creates gets permissions
   that keep other users out unless the mode begins with u; fopen's (and
   "u" modes') get the system's default ones (shared with the usual umask,
   022). Checked with stat(1). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <stdlib.h>

static void check(const char *name) {
    char cmd[160];
    fflush(stdout);
    snprintf(cmd, sizeof cmd,
             "if [ $(( 0$(stat -c %%a %s) & 077 )) -eq 0 ]; then echo %s private; else echo %s shared; fi", name, name,
             name);
    system(cmd);
    remove(name);
}

int main(void) {
    FILE *f;
    fopen_s(&f, "p1.txt", "w");
    fclose(f);
    check("p1.txt");
    fopen_s(&f, "p2.txt", "a+");
    fclose(f);
    check("p2.txt");
    fopen_s(&f, "p3.txt", "uw");
    fclose(f);
    check("p3.txt");
    f = fopen("p4.txt", "w");
    fclose(f);
    check("p4.txt");
    return 0;
}
// linux-only
// expect: p1.txt private
// expect: p2.txt private
// expect: p3.txt shared
// expect: p4.txt shared
