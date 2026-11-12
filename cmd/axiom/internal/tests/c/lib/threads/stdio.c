/* The standard I/O's stream locks under contention (C17 7.21.2p8: each
   function acts as if it took the stream's lock for its whole operation):
   16 threads write to three files at once, full buffered with a small
   buffer, line buffered and unbuffered, with fprintf (of several
   conversions, which the formatter writes in pieces), fputs, fwrite and
   putc; every line comes back whole and every byte is there. Then 8
   threads printf the same lines to standard output, which come out whole
   (the expected lines below, in any order: they are all the same). */
#include <stdio.h>
#include <string.h>
#include <threads.h>

#define THREADS 16
#define LINES 300

static FILE *f[3];

static int work(void *arg) {
    int id = (int)(long)arg, i;
    for (i = 0; i < LINES; i++) {
        char buf[96];
        FILE *o = f[i % 3];
        const char *p = buf;
        switch ((i / 3) % 4) {
        case 0:
            fprintf(o, "thread %2d line %3d: %s %5.2f %c end\n", id, i, "text", i / 4.0, 'x');
            break;
        case 1:
            sprintf(buf, "thread %2d line %3d: %s %5.2f %c end\n", id, i, "text", i / 4.0, 'x');
            fputs(buf, o);
            break;
        case 2:
            sprintf(buf, "thread %2d line %3d: %s %5.2f %c end\n", id, i, "text", i / 4.0, 'x');
            fwrite(buf, 1, strlen(buf), o);
            break;
        default:
            /* byte by byte: whole lines only under flockfile, which C does
               not have: here only the count of bytes is checked */
            sprintf(buf, "thread %2d line %3d: %s %5.2f %c end\n", id, i, "text", i / 4.0, 'x');
            while (*p)
                putc(*p++, o);
        }
    }
    return 0;
}

static int say(void *arg) {
    int i;
    (void)arg;
    for (i = 0; i < 4; i++)
        printf("%s %d %5.1f %-6s|%x\n", "the same line", 42, 2.5, "left", 255u);
    return 0;
}

int main(void) {
    thrd_t t[THREADS];
    int i, k;
    long lines = 0, whole = 0, bytes = 0, per = 0;
    for (k = 0; k < 3; k++)
        f[k] = tmpfile();
    setvbuf(f[0], NULL, _IOFBF, 64);
    setvbuf(f[1], NULL, _IOLBF, 128);
    setvbuf(f[2], NULL, _IONBF, 0);
    for (i = 0; i < THREADS; i++)
        thrd_create(&t[i], work, (void *)(long)i);
    for (i = 0; i < THREADS; i++)
        thrd_join(t[i], NULL);
    for (k = 0; k < 3; k++) {
        char line[200];
        int c;
        rewind(f[k]);
        while (fgets(line, sizeof line, f[k]) != NULL) {
            int id, n;
            char end[8];
            lines++;
            if (sscanf(line, "thread %d line %d: text %*f x %3s", &id, &n, end) == 3 && id >= 0 &&
                id < THREADS && strcmp(end, "end") == 0 && strlen(line) == 37)
                whole++;
        }
        rewind(f[k]);
        while ((c = getc(f[k])) != EOF)
            bytes++;
        fclose(f[k]);
    }
    /* the lines not written byte by byte are whole */
    for (i = 0; i < LINES; i++)
        per += (i / 3) % 4 != 3;
    printf("%ld %d %ld\n", bytes, whole >= per * THREADS, lines);
    fflush(stdout);
    for (i = 0; i < 8; i++)
        thrd_create(&t[i], say, NULL);
    for (i = 0; i < 8; i++)
        thrd_join(t[i], NULL);
    return 0;
}
/* expect: 177600 1 4800 */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
/* expect: the same line 42   2.5 left  |ff */
