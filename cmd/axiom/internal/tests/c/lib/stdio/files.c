/* Operations on files and file access (C17 7.21.4, 7.21.5, 7.21.7 to
   7.21.10) on real files in the working directory: every fopen mode (x
   and b included), reading after writing, appending, fseek and ftell,
   fgetpos and fsetpos, pushback, the end-of-file and error indicators,
   remove, rename, tmpfile, tmpnam, freopen, and the errors of each. */
#include <stdio.h>
#include <errno.h>

static void show(const char *name) {
    FILE *f = fopen(name, "rb");
    int c;
    putchar('<');
    if (f == NULL) {
        printf("missing>\n");
        return;
    }
    while ((c = getc(f)) != EOF)
        putchar(c == '\n' ? '/' : c);
    printf(">\n");
    fclose(f);
}

int main(void) {
    FILE *f;
    char buf[64];
    int c, e;
    long pos;
    fpos_t fp;
    size_t n;
    remove("a.txt");
    remove("b.txt");
    errno = 0;
    f = fopen("a.txt", "r");
    e = errno;
    printf("r missing: %d %d\n", f == NULL, e == ENOENT);
    printf("bad mode: %d\n", fopen("a.txt", "q") == NULL);
    /* w, wx (C11: fails if the file exists), a, a+ */
    f = fopen("a.txt", "wx");
    fputs("abc", f);
    fclose(f);
    f = fopen("a.txt", "wx");
    e = errno;
    printf("wx exists: %d %d\n", f == NULL, e == EEXIST);
    f = fopen("a.txt", "a");
    fputs("def", f);
    printf("append tell: %ld\n", ftell(f));
    fclose(f);
    f = fopen("a.txt", "a+");
    fputs("ghi", f);
    rewind(f);
    printf("a+ read: %s\n", fgets(buf, sizeof buf, f));
    /* (every write appends, whatever the position) */
    fseek(f, 0, SEEK_SET);
    fputs("j", f);
    fclose(f);
    show("a.txt");
    /* r+: writing in place, then reading after a seek */
    f = fopen("a.txt", "r+");
    fseek(f, 3, SEEK_SET);
    fputc('X', f);
    fseek(f, 0, SEEK_CUR);
    c = fgetc(f);
    pos = ftell(f);
    printf("r+: %c %ld\n", c, pos);
    fseek(f, -2, SEEK_END);
    printf("end-2: %c\n", fgetc(f));
    fgetpos(f, &fp);
    c = fgetc(f);
    c = fgetc(f);
    printf("eof: %d %d %d\n", c == EOF, feof(f) != 0, ferror(f) != 0);
    /* the end-of-file indicator stays set (7.21.7.1p3) */
    printf("again: %d\n", fgetc(f) == EOF);
    fsetpos(f, &fp);
    printf("fsetpos: %c %d\n", fgetc(f), feof(f) != 0);
    printf("seek before 0: %d\n", fseek(f, -100, SEEK_SET));
    printf("bad whence: %d\n", fseek(f, 0, 7));
    fclose(f);
    show("a.txt");
    /* w+: truncated; fread and fwrite; reading and writing in turn */
    f = fopen("a.txt", "w+b");
    printf("fwrite: %u\n", (unsigned)fwrite("0123456789", 2, 5, f));
    rewind(f);
    n = fread(buf, 3, 5, f);
    printf("fread: %u %d %d\n", (unsigned)n, feof(f) != 0, buf[8]);
    clearerr(f);
    fseek(f, 5, SEEK_SET);
    fwrite("ab", 1, 2, f);
    fseek(f, 0, SEEK_SET);
    n = fread(buf, 1, sizeof buf, f);
    buf[n] = 0;
    printf("w+: %s\n", buf);
    /* pushback: eight bytes, read last first; the position is the
       file's again after them */
    fseek(f, 2, SEEK_SET);
    for (c = 0; c < 8 && ungetc('a' + c, f) != EOF; c++)
        ;
    printf("pushed: %d ", c);
    while (c-- > 0)
        putchar(getc(f));
    printf(" %c %ld\n", getc(f), ftell(f));
    ungetc('Z', f);
    fseek(f, 0, SEEK_CUR);
    printf("seek drops it: %c\n", getc(f));
    /* reading a stream opened only for writing is an error */
    fclose(f);
    f = fopen("a.txt", "w");
    printf("read on w: %d %d\n", fgetc(f) == EOF, ferror(f) != 0);
    clearerr(f);
    printf("clearerr: %d\n", ferror(f));
    fclose(f);
    f = fopen("a.txt", "r");
    printf("write on r: %d %d\n", fputc('x', f) == EOF, ferror(f) != 0);
    fclose(f);
    /* rename, remove */
    printf("rename: %d\n", rename("a.txt", "b.txt"));
    show("a.txt");
    printf("remove: %d", remove("b.txt"));
    errno = 0;
    c = remove("b.txt");
    e = errno;
    printf(" %d %d\n", c, e == ENOENT);
    printf("rename missing: %d\n", rename("a.txt", "b.txt"));
    /* tmpfile: read and written, gone when closed */
    f = tmpfile();
    fprintf(f, "%d:%s\n", 99, "tmp");
    rewind(f);
    buf[0] = 0;
    c = 0;
    printf("tmpfile: %d", fscanf(f, "%d:%s", &c, buf));
    printf(" %d %s\n", c, buf);
    fclose(f);
    {
        char name[L_tmpnam];
        printf("tmpnam: %d", tmpnam(name) == name);
        f = fopen(name, "r");
        printf(" %d\n", f == NULL);
    }
    /* freopen: another file in the same stream */
    f = fopen("a.txt", "w");
    fputs("first", f);
    f = freopen("b.txt", "w", f);
    fputs("second", f);
    fclose(f);
    show("a.txt");
    show("b.txt");
    /* freopen with no name: the same file in another mode */
    f = fopen("b.txt", "rb");
    printf("mode change: %d", freopen(NULL, "r", f) == f);
    printf(" %c\n", getc(f));
    fclose(f);
    remove("a.txt");
    remove("b.txt");
    /* fgets: n - 1 characters, the last line without a new-line */
    f = tmpfile();
    fputs("one\ntwo\nlast", f);
    rewind(f);
    while (fgets(buf, 4, f) != NULL)
        printf("[%s]", buf[0] == '\n' ? "/" : buf);
    printf(" %d\n", feof(f) != 0);
    fclose(f);
    return 0;
}
/* expect: r missing: 1 1 */
/* expect: bad mode: 1 */
/* expect: wx exists: 1 1 */
/* expect: append tell: 6 */
/* expect: a+ read: abcdefghi */
/* expect: <abcdefghij> */
/* expect: r+: e 5 */
/* expect: end-2: i */
/* expect: eof: 1 1 0 */
/* expect: again: 1 */
/* expect: fsetpos: j 0 */
/* expect: seek before 0: -1 */
/* expect: bad whence: -1 */
/* expect: <abcXefghij> */
/* expect: fwrite: 5 */
/* expect: fread: 3 1 56 */
/* expect: w+: 01234ab789 */
/* expect: pushed: 8 hgfedcba 2 3 */
/* expect: seek drops it: 2 */
/* expect: read on w: 1 1 */
/* expect: clearerr: 0 */
/* expect: write on r: 1 1 */
/* expect: rename: 0 */
/* expect: <missing> */
/* expect: remove: 0 -1 1 */
/* expect: rename missing: -1 */
/* expect: tmpfile: 2 99 tmp */
/* expect: tmpnam: 1 1 */
/* expect: <first> */
/* expect: <second> */
/* expect: mode change: 1 s */
/* expect: [one][/][two][/][las][t] 1 */
