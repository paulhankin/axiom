/* A hash table of string keys probed in groups of 8 slots, with 8-bit tags and the key
   comparison through a function pointer (as a generic container makes it): hits, misses
   and inserts. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define GROUP 8
static inline int ctz32(uint32_t x) {
    int n = 0;
    while ((x & 1) == 0) { x >>= 1; n++; }
    return n;
}
typedef int (*eq_fn)(const char *, const char *, size_t);
typedef struct {
    uint8_t *tags;   /* 0: empty, else 0x80 | 7 bits of the hash */
    const char **keys;
    size_t *lens;
    uint32_t *vals;
    size_t groups;   /* a power of two */
    eq_fn eq;
} Table;
static int key_eq(const char *a, const char *b, size_t n) { return memcmp(a, b, n) == 0; }
static uint64_t hash_str(const char *s, size_t n) {
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < n; i++) h = (h ^ (uint8_t)s[i]) * 1099511628211ULL;
    return h ^ (h >> 29);
}
/* the slot of key k (hash h), or (size_t)-1; *free_at gets the first empty slot seen */
static size_t find(const Table *t, const char *k, size_t n, uint64_t h, size_t *free_at) {
    size_t mask = t->groups - 1, g = (size_t)h & mask, step = 0;
    uint8_t tag = (uint8_t)(0x80 | (h >> 57));
    for (;;) {
        const uint8_t *c = t->tags + g * GROUP;
        uint32_t m = 0, e = 0;
        for (int j = 0; j < GROUP; j++) {
            m |= (uint32_t)(c[j] == tag) << j;
            e |= (uint32_t)(c[j] == 0) << j;
        }
        while (m != 0) {
            size_t i = g * GROUP + (size_t)ctz32(m);
            if (t->lens[i] == n && t->eq(t->keys[i], k, n)) return i;
            m &= m - 1;
        }
        if (e != 0) {
            *free_at = g * GROUP + (size_t)ctz32(e);
            return (size_t)-1;
        }
        step++;
        g = (g + step) & mask;
    }
}
static char pool[1 << 22];
static size_t pool_used;
static const char *make_key(uint32_t x, size_t *n) {
    char *p = pool + pool_used;
    int len = sprintf(p, "key-%u-%x", x, x * 2654435761u);
    pool_used += (size_t)len + 1;
    *n = (size_t)len;
    return p;
}
int main(void) {
    Table t;
    t.groups = 1 << 14;
    size_t slots = t.groups * GROUP;
    t.tags = calloc(slots, 1);
    t.keys = calloc(slots, sizeof *t.keys);
    t.lens = calloc(slots, sizeof *t.lens);
    t.vals = calloc(slots, sizeof *t.vals);
    t.eq = key_eq;
    enum { N = 90000 };
    static const char *ks[N];
    static size_t ns[N];
    for (uint32_t i = 0; i < N; i++) ks[i] = make_key(i, &ns[i]);
    uint64_t sum = 0;
    for (uint32_t i = 0; i < N; i += 2) {
        uint64_t h = hash_str(ks[i], ns[i]);
        size_t fr = 0;
        if (find(&t, ks[i], ns[i], h, &fr) == (size_t)-1) {
            t.tags[fr] = (uint8_t)(0x80 | (h >> 57));
            t.keys[fr] = ks[i];
            t.lens[fr] = ns[i];
            t.vals[fr] = i;
        }
    }
    for (int r = 0; r < 40; r++) {
        for (uint32_t i = 0; i < N; i++) {
            uint64_t h = hash_str(ks[i], ns[i]);
            size_t fr = 0;
            size_t s = find(&t, ks[i], ns[i], h, &fr);
            sum += s == (size_t)-1 ? fr : t.vals[s];
        }
    }
    printf("callprobe: %llu\n", (unsigned long long)sum);
    return 0;
}
