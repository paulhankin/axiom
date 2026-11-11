/* flags: --c-std=c89 */
/* expect-warning: C4001 */
/* C89 in the old style: function definitions with identifier lists and
   parameter declarations (in any order, some left to default to int),
   implicit int for functions, objects and parameters, calls to functions
   declared without prototypes (the default argument promotions: char and
   short to int, float to double), implicit declarations of functions
   called before they are declared, declarations only at the start of
   blocks, register and auto, old-style function pointer declarations, and
   a text-processing workload: a word-frequency table with chained
   hashing, a line justifier, and run-length and move-to-front coding. */
#include "host.h"

#define NBUCKET 64
#define MAXWORDS 256

struct word {
    char text[16];
    int count;
    struct word *next;
};

static struct word words[MAXWORDS];
static nwords;                       /* implicit int */
static struct word *buckets[NBUCKET];
static unsigned long checksum = 0;

static void mix(v)
    unsigned long v;
{
    checksum = (checksum * 1000003UL ^ v) & 0xffffffffUL;
}

/* declared without a prototype; defined later with an identifier list */
static int scale();
double average();

static hash(s, n)
    char *s;
{                                    /* n defaults to int */
    register unsigned h = 0;
    register i;
    for (i = 0; i < n; i++) h = h * 31 + (unsigned char)s[i];
    return (int)(h % NBUCKET);
}

static int same(a, b, n) char *a, *b; int n;
{
    while (n-- > 0)
        if (*a++ != *b++) return 0;
    return *b == 0;
}

static struct word *add_word(s, n)
    char *s;
    int n;
{
    int b;
    struct word *w;
    if (n > 15) n = 15;
    b = hash(s, n);
    for (w = buckets[b]; w; w = w->next)
        if (same(s, w->text, n)) {
            w->count++;
            return w;
        }
    if (nwords == MAXWORDS) return 0;
    w = &words[nwords++];
    {
        int i;
        for (i = 0; i < n; i++) w->text[i] = s[i];
        w->text[n] = 0;
    }
    w->count = 1;
    w->next = buckets[b];
    buckets[b] = w;
    return w;
}

static is_letter(c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '\'';
}

static lower(c) int c;
{
    return c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c;
}

static count_words(text) char *text;
{
    char buf[64];
    int n = 0, total = 0;
    for (;; text++) {
        if (is_letter(*text)) {
            if (n < 63) buf[n++] = (char)lower(*text);
        } else {
            if (n) {
                add_word(buf, n);
                total++;
            }
            n = 0;
            if (!*text) break;
        }
    }
    return total;
}

/* a parameter declared float is passed as double and converted back */
static int scale(x, f, c, s)
    int x;
    float f;
    char c;
    short s;
{
    return (int)(x * f) + c + s;
}

double average(n, a)
    int n;
    int a[];
{
    auto int i;
    double s = 0;
    for (i = 0; i < n; i++) s += a[i];
    return n ? s / n : 0;
}

/* an old-style pointer to function and an array of them */
static int add1(x) { return x + 1; }
static int dbl(x) { return x * 2; }
static int neg(x) { return -x; }
static int (*ops[])() = { add1, dbl, neg, add1 };

static int apply_all(x, fs, n)
    int x;
    int (**fs)();
    int n;
{
    int i;
    for (i = 0; i < n; i++) x = (*fs[i])(x);
    return x;
}

/* full justification of a paragraph into lines of a width */
static justify(text, width, out) char *text; int width; char *out;
{
    char *words_at[128];
    int lens[128], nw = 0, i, lines = 0, pos = 0;
    char *p = text;
    while (*p) {
        while (*p == ' ') p++;
        if (!*p) break;
        words_at[nw] = p;
        while (*p && *p != ' ') p++;
        lens[nw] = (int)(p - words_at[nw]);
        nw++;
    }
    i = 0;
    while (i < nw) {
        int j = i, len = lens[i], gaps, spaces, k, extra;
        while (j + 1 < nw && len + 1 + lens[j + 1] <= width) {
            j++;
            len += 1 + lens[j];
        }
        gaps = j - i;
        spaces = width - (len - gaps);
        for (k = i; k <= j; k++) {
            int c;
            for (c = 0; c < lens[k]; c++) out[pos++] = words_at[k][c];
            if (k < j) {
                extra = j + 1 < nw ? spaces / gaps + (k - i < spaces % gaps) : 1;
                while (extra-- > 0) out[pos++] = ' ';
            }
        }
        if (j + 1 >= nw) while (pos % (width + 1) < width) out[pos++] = ' ';
        out[pos++] = '\n';
        lines++;
        i = j + 1;
    }
    out[pos] = 0;
    return lines;
}

/* run-length coding of bytes: (count, byte) pairs */
static rle(in, n, out) unsigned char *in, *out; int n;
{
    int i = 0, o = 0;
    while (i < n) {
        int j = i;
        while (j < n && in[j] == in[i] && j - i < 255) j++;
        out[o++] = (unsigned char)(j - i);
        out[o++] = in[i];
        i = j;
    }
    return o;
}

static unrle(in, n, out) unsigned char *in, *out;
{
    int i, o = 0;
    for (i = 0; i + 1 < n; i += 2) {
        int k = in[i];
        while (k-- > 0) out[o++] = in[i + 1];
    }
    return o;
}

/* move-to-front: small numbers for recently seen bytes */
static mtf(in, n, out) unsigned char *in, *out;
{
    unsigned char table[256];
    int i, sum = 0;
    for (i = 0; i < 256; i++) table[i] = (unsigned char)i;
    for (i = 0; i < n; i++) {
        int k = 0;
        unsigned char c = in[i];
        while (table[k] != c) k++;
        out[i] = (unsigned char)k;
        sum += k;
        while (k > 0) {
            table[k] = table[k - 1];
            k--;
        }
        table[0] = c;
    }
    return sum;
}

static char text[] =
    "It was a bright cold day in April and the clocks were striking thirteen and the day was cold "
    "and the wind was bright and the clocks were old. The day, the day, the DAY! Don't count twice "
    "what you can't count once; a word is a word is a word.";

static char justified[1024];
static unsigned char packed[1024], unpacked[1024], moved[1024];

main()
{
    int total, i, best = 0, lines, n, sum;
    struct word *w;
    char c = 'A';
    short s = -3;
    float f = 2.5f;
    int nums[7];
    static int init[] = { 3, 1, 4, 1, 5, 9, 2 };

    total = count_words(text);
    print_i64(total);
    print_i64(nwords);
    for (i = 0; i < nwords; i++) {
        w = &words[i];
        mix((unsigned long)w->count * 7 + (unsigned long)hash(w->text, 3));
        if (w->count > words[best].count) best = i;
    }
    print_str(words[best].text, 3);
    print_i64(words[best].count);
    print_u64(checksum);

    /* calls without prototypes: char, short promoted to int, float to double */
    print_i64(scale(10, f, c, s));
    print_i64(scale(7, 0.25, 'b', 1000));
    for (i = 0; i < 7; i++) nums[i] = init[i] * 10;
    print_f64(average(7, nums));
    print_i64(apply_all(5, ops, 4));
    print_i64(later(6, 7));          /* implicitly declared: int later() */

    lines = justify("the quick brown fox jumps over the lazy dog and keeps running far away from here", 24, justified);
    print_i64(lines);
    for (i = 0; justified[i]; i++) mix((unsigned long)(unsigned char)justified[i]);
    print_str(justified, 24);
    print_u64(checksum);

    for (i = 0; i < 300; i++) unpacked[i] = (unsigned char)(i < 100 ? 'a' : i < 160 ? 'b' : i % 3 ? 'c' : 'd');
    n = rle(unpacked, 300, packed);
    print_i64(n);
    print_i64(unrle(packed, n, moved));
    sum = 0;
    for (i = 0; i < 300; i++) sum += moved[i] == unpacked[i];
    print_i64(sum);
    print_i64(mtf((unsigned char *)text, (int)sizeof text - 1, moved));
    return 0;
}

later(a, b)
{
    return a * b + nwords;
}

/* expect: 51 */
/* expect: 25 */
/* expect: the */
/* expect: 7 */
/* expect: 3540366997 */
/* expect: 87 */
/* expect: 1099 */
/* expect: 35.714285714285715 */
/* expect: -11 */
/* expect: 67 */
/* expect: 4 */
/* expect: the   quick   brown  fox */
/* expect: 980279785 */
/* expect: 190 */
/* expect: 300 */
/* expect: 300 */
/* expect: 4735 */
