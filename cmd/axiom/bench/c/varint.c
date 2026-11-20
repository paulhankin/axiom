/* Record encoding as in a database's pages: varints written and read back, records compared. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
static int put_varint(uint8_t *p, uint64_t v) {
    if (v <= 0x7f) { p[0] = (uint8_t)v; return 1; }
    uint8_t tmp[10]; int n = 0;
    while (v) { tmp[n++] = (uint8_t)((v & 0x7f) | 0x80); v >>= 7; }
    tmp[0] &= 0x7f;
    for (int i = 0; i < n; i++) p[i] = tmp[n - 1 - i];
    return n;
}
static int get_varint(const uint8_t *p, uint64_t *v) {
    uint64_t x = 0; int i = 0;
    do { x = (x << 7) | (p[i] & 0x7f); } while (p[i++] & 0x80);
    *v = x; return i;
}
typedef struct { uint64_t rowid; int ncol; uint64_t col[6]; } Rec;
static int rec_cmp(const Rec *a, const Rec *b) {
    for (int i = 0; i < a->ncol && i < b->ncol; i++) {
        if (a->col[i] != b->col[i]) return a->col[i] < b->col[i] ? -1 : 1;
    }
    return a->ncol - b->ncol;
}
int main(void) {
    size_t cap = 8 << 20;
    uint8_t *page = malloc(cap);
    uint64_t s = 3, h = 0;
    for (int rep = 0; rep < 18; rep++) {
        size_t p = 0; int nrec = 0;
        while (p + 80 < cap) {
            s = s * 6364136223846793005ULL + 1442695040888963407ULL;
            int ncol = 1 + (int)((s >> 60) % 6);
            p += put_varint(page + p, (uint64_t)nrec);
            page[p++] = (uint8_t)ncol;
            for (int c = 0; c < ncol; c++) p += put_varint(page + p, (s >> (c * 9)) & ((1ULL << (c * 7 + 5)) - 1));
            nrec++;
        }
        size_t q = 0; Rec prev; memset(&prev, 0, sizeof prev);
        for (int i = 0; i < nrec; i++) {
            Rec r;
            q += get_varint(page + q, &r.rowid);
            r.ncol = page[q++];
            for (int c = 0; c < r.ncol; c++) q += get_varint(page + q, &r.col[c]);
            h += (uint64_t)(rec_cmp(&r, &prev) + 1) + r.col[0];
            prev = r;
        }
    }
    printf("varint: %llu\n", (unsigned long long)h);
    return 0;
}
