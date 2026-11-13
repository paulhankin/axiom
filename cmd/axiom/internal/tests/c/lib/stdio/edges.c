/* Edge cases of the file access functions: fflush(NULL) writes every
   output stream (C17 7.21.5.2p3); freopen with a null file name changes
   the mode of the same file (7.21.5.4p3: here a mode that needs what the
   file was not opened for fails and closes the stream); setvbuf's modes,
   a buffer of the program's and its errors (7.21.5.6); rename over an
   existing file replaces it (7.21.4.2p2 leaves it implementation-defined:
   every target replaces); strtod reads neither C23's digit separators nor
   a 0b prefix (7.24.1.5: they are not part of the subject sequence). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void show(const char *name) {
    FILE *f = fopen(name, "r");
    char buf[64];
    size_t n = f != NULL ? fread(buf, 1, sizeof buf - 1, f) : 0;
    buf[n] = 0;
    printf("%s: [%s]\n", name, f != NULL ? buf : "missing");
    if (f != NULL)
        fclose(f);
}

int main(void) {
    FILE *a = fopen("a.txt", "w"), *b = fopen("b.txt", "w"), *f;
    char ub[4], *end;
    int r;
    fputs("aaa", a);
    fputs("bbb", b);
    show("a.txt");
    printf("fflush(NULL) %d\n", fflush(NULL));
    show("a.txt");
    show("b.txt");
    fclose(a);
    fclose(b);

    /* freopen(NULL, ...) */
    f = fopen("a.txt", "r+");
    f = freopen(NULL, "a", f);
    fputs("+appended", f);
    f = freopen(NULL, "rb", f);
    rewind(f);
    printf("reread %c\n", fgetc(f));
    fclose(f);
    show("a.txt");
    f = fopen("a.txt", "r");
    printf("r to w %d\n", freopen(NULL, "w", f) == NULL);

    /* setvbuf */
    f = fopen("b.txt", "w");
    printf("bad mode %d\n", setvbuf(f, NULL, 7, 0) != 0);
    printf("own buffer %d\n", setvbuf(f, ub, _IOFBF, sizeof ub));
    fputs("12", f);
    show("b.txt");
    fputs("345", f);
    show("b.txt");
    printf("unbuffered %d\n", setvbuf(f, NULL, _IONBF, 0));
    fputs("6", f);
    show("b.txt");
    printf("line %d\n", setvbuf(f, NULL, _IOLBF, 0));
    fputs("7\n8", f);
    show("b.txt");
    fclose(f);
    show("b.txt");

    /* rename over an existing file */
    printf("rename %d\n", rename("a.txt", "b.txt"));
    show("a.txt");
    show("b.txt");
    remove("b.txt");

    /* strtod: no digit separators, no 0b */
    printf("%g %s\n", strtod("1'000", &end), end);
    printf("%g %s\n", strtod("0b101", &end), end);
    printf("%g %s\n", strtod("0x1'0p0", &end), end);
    r = sscanf("1'5", "%lf", &(double){0});
    printf("sscanf %d\n", r);
    return 0;
}

// expect: a.txt: []
// expect: fflush(NULL) 0
// expect: a.txt: [aaa]
// expect: b.txt: [bbb]
// expect: reread a
// expect: a.txt: [aaa+appended]
// expect: r to w 1
// expect: bad mode 1
// expect: own buffer 0
// expect: b.txt: []
// expect: b.txt: [1234]
// expect: unbuffered 0
// expect: b.txt: [123456]
// expect: line 0
// expect: b.txt: [1234567
// expect: ]
// expect: b.txt: [1234567
// expect: 8]
// expect: rename 0
// expect: a.txt: [missing]
// expect: b.txt: [aaa+appended]
// expect: 1 '000
// expect: 0 b101
// expect: 1 '0p0
// expect: sscanf 1
