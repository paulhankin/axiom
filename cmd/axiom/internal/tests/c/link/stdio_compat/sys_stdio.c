#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void sys_print(const char *s) {
    char buf[32];
    snprintf(buf, sizeof buf, "<%s>", s);
    fputs(buf, stdout);
    fprintf(stdout, " %d\n", (int)strlen(buf));
}
int sys_errno(void) {
    errno = 0;
    strtol("99999999999999999999999", 0, 10);
    return errno == ERANGE;
}
int sys_ctype(const char *s) {
    int n = 0;
    for (; *s; s++)
        if (isalnum((unsigned char)*s) && toupper((unsigned char)*s) == *s)
            n++;
    return n;
}
static int cmp(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }
int sys_sort(void) {
    int v[5] = {5, 3, 9, 1, 4};
    qsort(v, 5, sizeof v[0], cmp);
    return v[0] * 10000 + v[1] * 1000 + v[2] * 100 + v[3] * 10 + v[4];
}
static jmp_buf jb;
static void deep(int v) { longjmp(jb, v); }
int sys_jump(int v) {
    int r = setjmp(jb);
    if (r == 0)
        deep(v);
    return r;
}
void sys_assert(int x) { assert(x > 0); }
