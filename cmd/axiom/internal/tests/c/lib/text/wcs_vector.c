/* wcslen, wmemchr and wmemcmp search many wide characters at a time
   (std/c/rt/text_wide.ax): their results agree with element-by-element
   searches for every alignment of the start and every length up to 300,
   for characters with high bits set, and for a string that ends at the end
   of its allocation (the whole aligned groups read past its null do not
   leave its page). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static size_t naive_len(const wchar_t *s) {
    size_t n = 0;
    while (s[n] != 0)
        n++;
    return n;
}

int main(void) {
    static wchar_t buf[400];
    size_t off, len, k, bad = 0, checks = 0;
    wchar_t big = (wchar_t)(WCHAR_MAX > 0xFFFF ? 0x10FFFF : 0xFFFF);
    for (off = 0; off < 16; off++)
        for (len = 0; len < 300; len++) {
            wchar_t *s = buf + off;
            for (k = 0; k < len; k++)
                s[k] = (wchar_t)(k % 7 == 3 ? big : 'a' + (int)(k % 23));
            s[len] = 0;
            s[len + 1] = 'z';
            checks++;
            if (wcslen(s) != naive_len(s) || wcslen(s) != len)
                bad++;
            for (k = 0; k <= len; k += 1 + len / 9) {
                wchar_t save = s[k], *p;
                s[k] = (wchar_t)0x7FFE;
                p = wmemchr(s, (wchar_t)0x7FFE, len + 1);
                checks++;
                if (p != s + k)
                    bad++;
                {
                    wchar_t t[400];
                    memcpy(t, s, (len + 1) * sizeof(wchar_t));
                    t[k] = save;
                    checks++;
                    if (wmemcmp(s, t, len + 1) != (s[k] < save ? -1 : 1) || wmemcmp(s, s, len + 1) != 0)
                        bad++;
                }
                s[k] = save;
            }
            checks++;
            if (wmemchr(s, (wchar_t)0x7FFE, len) != NULL)
                bad++;
        }
    /* a string that ends at the end of its block */
    {
        size_t n;
        for (n = 1; n < 70; n++) {
            wchar_t *e = malloc(n * sizeof(wchar_t));
            for (k = 0; k + 1 < n; k++)
                e[k] = 'x';
            e[n - 1] = 0;
            checks++;
            if (wcslen(e) != n - 1)
                bad++;
            free(e);
        }
    }
    printf("%d problems\n", (int)bad);
    return checks > 10000 ? 0 : 1;
}
// expect: 0 problems
