/* String scanning: tokenising text into words and numbers, counting and hashing them. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
static int isalpha_(int c) { return (unsigned)((c | 32) - 'a') < 26; }
static int isdigit_(int c) { return (unsigned)(c - '0') < 10; }
int main(void) {
    size_t n = 4 << 20;
    char *buf = malloc(n + 1);
    const char *words[] = {"select", "from", "where", "id", "=", "42", "and", "name", "like", "'abc%'", ",", "(", ")", "12345", "x1"};
    uint32_t s = 1;
    size_t p = 0;
    while (p < n - 16) {
        s = s * 1103515245 + 12345;
        const char *w = words[(s >> 16) % 15];
        size_t l = strlen(w);
        memcpy(buf + p, w, l); p += l;
        buf[p++] = ((s >> 8) & 7) == 0 ? '\n' : ' ';
    }
    buf[p] = 0;
    uint64_t h = 0, nwords = 0, nnums = 0, numsum = 0, lines = 0;
    for (int rep = 0; rep < 24; rep++) {
        const char *q = buf;
        while (*q) {
            char c = *q;
            if (isalpha_(c)) {
                uint32_t wh = 2166136261u;
                while (isalpha_(*q) || isdigit_(*q)) { wh = (wh ^ (uint8_t)*q) * 16777619u; q++; }
                h += wh; nwords++;
            } else if (isdigit_(c)) {
                uint64_t v = 0;
                while (isdigit_(*q)) { v = v * 10 + (*q - '0'); q++; }
                numsum += v; nnums++;
            } else if (c == '\'') {
                q++;
                while (*q && *q != '\'') q++;
                if (*q) q++;
            } else {
                if (c == '\n') lines++;
                q++;
            }
        }
        h += strlen(buf) + (strchr(buf + rep, 'x') - buf);
    }
    printf("strscan: %llu %llu %llu %llu %llu\n", (unsigned long long)h, (unsigned long long)nwords,
           (unsigned long long)nnums, (unsigned long long)numsum, (unsigned long long)lines);
    return 0;
}
