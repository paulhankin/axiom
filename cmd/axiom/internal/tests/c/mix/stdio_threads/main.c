/* A stream's lock (C17 7.21.2p8): threads (the Axiom runtime's, started
   by rt.ax) write to two streams at once: lines with fprintf and fputs to
   one, which come out whole, and bytes with putc to the other, none of
   which is lost. */
#include <stdio.h>

void rt_run_threads(int n, void (*work)(int));

static FILE *lines_out, *bytes_out;

static void work(int id) {
    int i;
    for (i = 0; i < 500; i++) {
        char buf[64];
        const char *p = buf;
        if (i % 3 == 0) {
            fprintf(lines_out, "thread %d line %d end\n", id, i);
            continue;
        }
        sprintf(buf, "thread %d line %d end\n", id, i);
        if (i % 3 == 1) {
            fputs(buf, lines_out);
            continue;
        }
        while (*p)
            putc(*p++, bytes_out);
    }
}

int main(void) {
    char line[128];
    long lines = 0, whole = 0, bytes = 0;
    lines_out = tmpfile();
    bytes_out = tmpfile();
    setvbuf(lines_out, NULL, _IOFBF, 256);
    setvbuf(bytes_out, NULL, _IOFBF, 100);
    rt_run_threads(4, work);
    rewind(lines_out);
    while (fgets(line, sizeof line, lines_out) != NULL) {
        int id, i;
        char end[8];
        lines++;
        if (sscanf(line, "thread %d line %d %3s", &id, &i, end) == 3 && id >= 0 && id < 4 && i % 3 != 2)
            whole++;
    }
    rewind(bytes_out);
    while (getc(bytes_out) != EOF)
        bytes++;
    /* 4 x 334 lines; 4 x 166 lines of 21 or 22 bytes */
    printf("%ld %ld %ld\n", lines, whole, bytes);
    fclose(lines_out);
    fclose(bytes_out);
    return 0;
}
/* expect: 1336 1336 14464 */
