/* A text processor over the hosted library: reads its standard input a line
   at a time (fgets), splits words (strtok), folds case (tolower), counts
   them in a hash table, sorts by count then word (qsort), and prints a
   report with fprintf's widths and precisions; then the same text as wide
   characters in a UTF-8 locale (mbstowcs, iswalpha, towupper), written to a
   wide-oriented file (fwprintf, fputwc) and read back (fgetws, wcstol),
   with sprintf/sscanf round trips of every count. */
#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#define BUCKETS 211

struct word {
    char *text;
    int count;
    int first_line;
    struct word *next;
};

static struct word *table[BUCKETS];
static int nwords, ndistinct;

static unsigned hash(const char *s) {
    unsigned h = 5381;
    while (*s)
        h = h * 33u + (unsigned char)*s++;
    return h % BUCKETS;
}

static void add(const char *w, int line) {
    unsigned h = hash(w);
    struct word *p;
    for (p = table[h]; p != NULL; p = p->next)
        if (strcmp(p->text, w) == 0) {
            p->count++;
            return;
        }
    p = malloc(sizeof *p);
    p->text = malloc(strlen(w) + 1);
    strcpy(p->text, w);
    p->count = 1;
    p->first_line = line;
    p->next = table[h];
    table[h] = p;
    ndistinct++;
}

static int by_count(const void *a, const void *b) {
    const struct word *x = *(const struct word *const *)a, *y = *(const struct word *const *)b;
    if (x->count != y->count)
        return y->count - x->count;
    return strcmp(x->text, y->text);
}

int main(void) {
    char line[256], all[4096] = "";
    int lineno = 0, i, longest = 0, letters = 0, digits = 0, punct = 0;
    struct word **list;
    size_t k;
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *w;
        lineno++;
        strncat(all, line, sizeof all - strlen(all) - 1);
        for (k = 0; line[k] != 0; k++) {
            unsigned char c = (unsigned char)line[k];
            letters += isalpha(c) != 0;
            digits += isdigit(c) != 0;
            punct += ispunct(c) != 0;
        }
        for (w = strtok(line, " \t\n.,;:!?\"()"); w != NULL; w = strtok(NULL, " \t\n.,;:!?\"()")) {
            char *p;
            if ((unsigned char)*w >= 0x80)
                continue; /* (the wide part counts those) */
            for (p = w; *p; p++)
                *p = (char)tolower((unsigned char)*p);
            if ((int)strlen(w) > longest)
                longest = (int)strlen(w);
            add(w, lineno);
            nwords++;
        }
    }
    printf("%d lines, %d words, %d distinct, longest %d\n", lineno, nwords, ndistinct, longest);
    printf("letters %d, digits %d, punctuation %d\n", letters, digits, punct);
    list = malloc((size_t)ndistinct * sizeof *list);
    k = 0;
    for (i = 0; i < BUCKETS; i++) {
        struct word *p;
        for (p = table[i]; p != NULL; p = p->next)
            list[k++] = p;
    }
    qsort(list, k, sizeof *list, by_count);
    for (i = 0; i < 12 && i < ndistinct; i++)
        printf("%2d. %-*s %3d %5.1f%% (line %d)\n", i + 1, longest, list[i]->text, list[i]->count,
               100.0 * list[i]->count / nwords, list[i]->first_line);
    /* round trips of the counts through the formatted functions */
    {
        int bad = 0;
        for (i = 0; i < ndistinct; i++) {
            char buf[64], back[32];
            int n;
            sprintf(buf, "%s=%d;", list[i]->text, list[i]->count);
            if (sscanf(buf, "%31[^=]=%d;", back, &n) != 2 || n != list[i]->count || strcmp(back, list[i]->text))
                bad++;
        }
        printf("round trips: %d bad\n", bad);
    }
    /* the wide part */
    if (setlocale(LC_ALL, "C.UTF-8") != NULL) {
        static wchar_t w[4096];
        size_t n = mbstowcs(w, all, 4096), alpha = 0, upper = 0, nonascii = 0;
        FILE *f = fopen("wide.txt", "w+");
        wchar_t back[256];
        long total = 0;
        for (k = 0; k < n; k++) {
            alpha += iswalpha((wint_t)w[k]) != 0;
            upper += iswupper((wint_t)w[k]) != 0;
            nonascii += w[k] >= 0x80;
        }
        printf("wide: %d characters, %d alphabetic, %d upper case, %d beyond ASCII\n", (int)n, (int)alpha, (int)upper,
               (int)nonascii);
        for (k = 0; k < n; k++)
            if (w[k] >= 0x80 && iswalpha((wint_t)w[k])) {
                fputwc((wchar_t)towupper((wint_t)w[k]), f);
            }
        fwprintf(f, L"\n%d %ls\n", (int)nonascii, L"done");
        rewind(f);
        if (fgetws(back, 256, f) != NULL) {
            back[wcscspn(back, L"\n")] = 0;
            printf("upper: %ls\n", back);
        }
        if (fgetws(back, 256, f) != NULL) {
            wchar_t *end;
            total = wcstol(back, &end, 10);
            printf("read back %ld,%ls", total, end);
        }
        fclose(f);
        remove("wide.txt");
    }
    for (i = 0; i < ndistinct; i++) {
        free(list[i]->text);
        free(list[i]);
    }
    free(list);
    return 0;
}
// stdin: The quick brown fox jumps over the lazy dog. The dog sleeps;
// stdin: the fox runs: 42 times, then 7 more (quickly!) over the hill.
// stdin: "Über alles" sagte der Fuchs, und der Hund schlief weiter.
// stdin: Ça va? Très bien, merci. The end of the text, the end.

// expect: 4 lines, 44 words, 32 distinct, longest 7
// expect: letters 168, digits 3, punctuation 17
// expect:  1. the       8  18.2% (line 1)
// expect:  2. der       2   4.5% (line 3)
// expect:  3. dog       2   4.5% (line 1)
// expect:  4. end       2   4.5% (line 4)
// expect:  5. fox       2   4.5% (line 1)
// expect:  6. over      2   4.5% (line 1)
// expect:  7. 42        1   2.3% (line 2)
// expect:  8. 7         1   2.3% (line 2)
// expect:  9. alles     1   2.3% (line 3)
// expect: 10. bien      1   2.3% (line 4)
// expect: 11. brown     1   2.3% (line 1)
// expect: 12. fuchs     1   2.3% (line 3)
// expect: round trips: 0 bad
// expect: wide: 237 characters, 171 alphabetic, 8 upper case, 3 beyond ASCII
// expect: upper: ÜÇÈ
// expect: read back 3, done
