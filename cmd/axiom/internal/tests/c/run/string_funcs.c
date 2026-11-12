/* <string.h>: each function C23 asks of a freestanding implementation
   (std/c/lib/string/), with the cases at its edges: empty strings, the
   null byte, bytes above 0x7f, and every length and alignment up to a few
   words (the word-at-a-time paths) against byte-by-byte references. */
#include "host.h"
#include <string.h>

static unsigned char a[300], b[300];
static unsigned seed = 1;

static unsigned rnd(void) {
    seed = seed * 1103515245u + 12345u;
    return (seed >> 16) & 0x7fff;
}

static int sign(int x) { return (x > 0) - (x < 0); }

int main(void) {
    char buf[64];
    /* copying and concatenation */
    print_i64(strcpy(buf, "hello") == buf);
    print_i64(strcat(buf, ", world") == buf);
    print_str(buf, strlen(buf));
    print_i64(strlen(""));
    strncpy(buf, "ab", 5);
    print_i64(buf[0] + buf[1] + buf[2] + buf[3] + buf[4]);
    strncpy(buf, "abcdef", 3);
    print_i64(buf[2] == 'c' && buf[3] == 0);
    strcpy(buf, "x");
    strncat(buf, "yzw", 2);
    print_str(buf, strlen(buf));
    /* comparison */
    print_i64(sign(strcmp("abc", "abd")) * 100 + sign(strcmp("b", "a")) * 10 + strcmp("", ""));
    print_i64(sign(strcmp("\xff", "a")));
    print_i64(strncmp("abcx", "abcy", 3) + sign(strncmp("abcx", "abcy", 4)) * 10);
    print_i64(sign(memcmp("\x80", "\x7f", 1)) + memcmp("a", "b", 0));
    /* search */
    const char *s = "one two three";
    print_i64(strchr(s, 't') - s);
    print_i64(strrchr(s, 't') - s);
    print_i64(strchr(s, 0) - s);
    print_i64(strchr(s, 'z') == NULL);
    print_i64(strstr(s, "two") - s);
    print_i64(strstr(s, "") == s);
    print_i64(strstr(s, "twx") == NULL);
    print_i64(strspn(s, "eno") * 10 + strcspn(s, " "));
    print_i64(strpbrk(s, "wx") - s);
    print_i64(strpbrk(s, "xyz") == NULL);
    print_i64((const char *)memchr(s, 'w', 13) - s);
    print_i64(memchr(s, 'w', 5) == NULL);
    char text[] = "  a,b;;c  ";
    char *t = strtok(text, " ,;");
    while (t != NULL) {
        print_str(t, strlen(t));
        t = strtok(NULL, " ,;");
    }
    char dst[10] = "abc;....";
    memset(dst, 'q', 3);
    print_str(dst, 8);
    /* every length and alignment against references */
    int bad = 0;
    for (int k = 0; k < 2000; k++) {
        size_t n = rnd() % 70, i = rnd() % 16, j = rnd() % 16;
        for (size_t q = 0; q < 300; q++) {
            a[q] = (unsigned char)rnd();
            b[q] = (unsigned char)rnd();
        }
        unsigned char save[300];
        memcpy(save, a, 300);
        memcpy(a + i, b + j, n);
        for (size_t q = 0; q < 300; q++) {
            unsigned char want = (q >= i && q < i + n) ? b[j + q - i] : save[q];
            bad += a[q] != want;
        }
        memcpy(save, a, 300);
        memmove(a + i, a + j, n);
        for (size_t q = 0; q < 300; q++) {
            unsigned char want = (q >= i && q < i + n) ? save[j + q - i] : save[q];
            bad += a[q] != want;
        }
        memcpy(save, a, 300);
        memset(a + i, (int)(n * 7), n);
        for (size_t q = 0; q < 300; q++) {
            bad += a[q] != ((q >= i && q < i + n) ? (unsigned char)(n * 7) : save[q]);
        }
        memcpy(b + j, a + i, n);
        if (n > 0 && (rnd() & 1)) {
            b[j + rnd() % n] ^= 1 + rnd() % 255;
        }
        int want = 0;
        for (size_t q = 0; q < n; q++) {
            if (a[i + q] != b[j + q]) {
                want = a[i + q] < b[j + q] ? -1 : 1;
                break;
            }
        }
        bad += sign(memcmp(a + i, b + j, n)) != want;
        a[i + n] = 0;
        for (size_t q = 0; q < n; q++) {
            if (a[i + q] == 0) {
                a[i + q] = 1;
            }
        }
        bad += strlen((char *)a + i) != n;
        unsigned char c = (unsigned char)rnd();
        const unsigned char *hit = NULL;
        for (size_t q = 0; q < n; q++) {
            if (a[i + q] == c) {
                hit = a + i + q;
                break;
            }
        }
        bad += memchr(a + i, c, n) != hit;
    }
    print_i64(bad);
    return 0;
}
/* expect: 1 */
/* expect: 1 */
/* expect: hello, world */
/* expect: 0 */
/* expect: 195 */
/* expect: 1 */
/* expect: xyz */
/* expect: -90 */
/* expect: 1 */
/* expect: -10 */
/* expect: 1 */
/* expect: 4 */
/* expect: 8 */
/* expect: 13 */
/* expect: 1 */
/* expect: 4 */
/* expect: 1 */
/* expect: 1 */
/* expect: 33 */
/* expect: 5 */
/* expect: 1 */
/* expect: 5 */
/* expect: 1 */
/* expect: a */
/* expect: b */
/* expect: c */
/* expect: qqq;.... */
/* expect: 0 */
