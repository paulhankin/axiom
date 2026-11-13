/* Annex K's <stdio.h> (C11 K.3.5): the formatted output functions (%n and
   null %s pointers are violations; sprintf_s's output must fit, snprintf_s
   truncates), the formatted input functions (c, s and [ take sizes),
   gets_s, fopen_s, freopen_s, tmpfile_s and tmpnam_s. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void handler(const char *restrict msg, void *restrict ptr, errno_t error) {
    (void)ptr;
    printf("handler: %s (%d)\n", msg, error != 0);
}

int main(void) {
    char s[8], t[4], name[L_tmpnam_s], line[6];
    int r, n = 0, x = 0;
    errno_t e;
    FILE *f, *g;
    set_constraint_handler_s(handler);

    r = printf_s("printf_s %d %s\n", 42, "ok");
    printf("-> %d\n", r);
    fflush(stdout);
    r = printf_s("before %n after\n", &n);
    printf("\n%%n: %d\n", r);
    r = printf_s("x %s\n", (char *)NULL);
    printf("null %%s: %d\n", r);
    r = printf_s("%%n is text here\n");
    printf("-> %d\n", r);
    r = fprintf_s(NULL, "x");
    printf("null stream: %d\n", r);

    r = snprintf_s(s, sizeof s, "%d-%s", 12345, "abcdef");
    printf("snprintf_s %d [%s]\n", r, s);
    r = snprintf_s(s, 0, "x");
    printf("snprintf_s 0: %d\n", r);
    r = snprintf_s(s, sizeof s, "%n", &n);
    printf("snprintf_s %%n: %d [%s]\n", r, s);
    r = sprintf_s(s, sizeof s, "%d", 1234567);
    printf("sprintf_s fits %d [%s]\n", r, s);
    r = sprintf_s(s, sizeof s, "%d", 12345678);
    printf("sprintf_s too long %d [%s]\n", r, s);
    r = sprintf_s(s, sizeof s, "%s", (char *)NULL);
    printf("sprintf_s null %d\n", r);

    /* sscanf_s: the sizes of c, s and [ arrays */
    r = sscanf_s("abc 12 xyz", "%s %d %3c", s, (rsize_t)sizeof s, &x, t, (rsize_t)3);
    printf("sscanf_s %d [%s] %d [%.3s]\n", r, s, x, t);
    strcpy(t, "zz");
    r = sscanf_s("hello", "%s", t, (rsize_t)sizeof t);
    printf("sscanf_s too long %d [%s]\n", r, t);
    r = sscanf_s("abcd", "%4c", t, (rsize_t)3);
    printf("sscanf_s %%c too long %d\n", r);
    r = sscanf_s("aab!", "%[ab]%n", t, (rsize_t)sizeof t, &n);
    printf("sscanf_s [ %d [%s] %d\n", r, t, n);
    r = sscanf_s("abc", "%*s%n", &n);
    printf("sscanf_s suppressed %d %d\n", r, n);
    r = sscanf_s("12", "%d", (int *)NULL);
    printf("sscanf_s null %d\n", r == EOF);
    r = sscanf_s(NULL, "%d", &x);
    printf("sscanf_s null string %d\n", r == EOF);

    /* gets_s: lines of the standard input */
    printf("gets_s [%s]\n", gets_s(line, sizeof line));
    printf("gets_s too long %d [%s]\n", gets_s(line, sizeof line) == NULL, line);
    printf("gets_s after [%s]\n", gets_s(line, sizeof line));
    printf("gets_s end %d\n", gets_s(line, sizeof line) == NULL);

    /* fopen_s, freopen_s */
    e = fopen_s(&f, "k.txt", "w");
    printf("fopen_s w %d\n", e);
    fputs("ext1", f);
    fclose(f);
    e = fopen_s(&f, "k.txt", "ur");
    printf("fopen_s ur %d %c\n", e, fgetc(f));
    e = freopen_s(&g, NULL, "rb", f);
    printf("freopen_s same %d %d %c\n", e, g == f, fgetc(f));
    fclose(f);
    e = fopen_s(&f, "missing-dir/k.txt", "r");
    printf("fopen_s missing %d %d\n", e == ENOENT, f == NULL);
    e = fopen_s(&f, NULL, "r");
    printf("fopen_s null %d %d\n", e != 0, f == NULL);
    e = freopen_s(&g, "k.txt", NULL, stdout);
    printf("freopen_s null mode %d %d\n", e != 0, g == NULL);
    remove("k.txt");

    /* tmpfile_s, tmpnam_s */
    e = tmpfile_s(&f);
    printf("tmpfile_s %d\n", e);
    fputs("tmp", f);
    rewind(f);
    printf("read back %c\n", fgetc(f));
    fclose(f);
    e = tmpfile_s(NULL);
    e = tmpnam_s(name, sizeof name);
    printf("tmpnam_s %d %d\n", e, strlen(name) > 12);
    e = tmpnam_s(s, sizeof s);
    printf("tmpnam_s too small %d [%s]\n", e != 0, s);
    return 0;
}
// stdin: line1
// stdin: toolongline
// stdin: next

// expect: printf_s 42 ok
// expect: -> 15
// expect: before handler: printf_s: %n in the format (1)
// expect:
// expect: %n: -1
// expect: x handler: printf_s: a null pointer for %s (1)
// expect: null %s: -1
// expect: %n is text here
// expect: -> 16
// expect: handler: fprintf_s: stream is a null pointer (1)
// expect: null stream: -1
// expect: snprintf_s 12 [12345-a]
// expect: handler: snprintf_s: n is zero (1)
// expect: snprintf_s 0: -1
// expect: handler: snprintf_s: %n in the format (1)
// expect: snprintf_s %n: -1 []
// expect: sprintf_s fits 7 [1234567]
// expect: handler: sprintf_s: the output does not fit in n characters (1)
// expect: sprintf_s too long 0 []
// expect: handler: sprintf_s: a null pointer for %s (1)
// expect: sprintf_s null 0
// expect: sscanf_s 3 [abc] 12 [xyz]
// expect: sscanf_s too long 0 []
// expect: sscanf_s %c too long 0
// expect: sscanf_s [ 1 [aab] 3
// expect: sscanf_s suppressed 0 3
// expect: handler: sscanf_s: a null pointer to store converted input (1)
// expect: sscanf_s null 1
// expect: handler: sscanf_s: s is a null pointer (1)
// expect: sscanf_s null string 1
// expect: gets_s [line1]
// expect: handler: gets_s: the line is longer than n - 1 characters (1)
// expect: gets_s too long 1 []
// expect: gets_s after [next]
// expect: gets_s end 1
// expect: fopen_s w 0
// expect: fopen_s ur 0 e
// expect: freopen_s same 0 1 x
// expect: fopen_s missing 1 1
// expect: handler: fopen_s: filename is a null pointer (1)
// expect: fopen_s null 1 1
// expect: handler: freopen_s: mode is a null pointer (1)
// expect: freopen_s null mode 1 1
// expect: tmpfile_s 0
// expect: read back t
// expect: handler: tmpfile_s: streamptr is a null pointer (1)
// expect: tmpnam_s 0 1
// expect: handler: tmpnam_s: the name does not fit in maxsize characters (1)
// expect: tmpnam_s too small 1 []
