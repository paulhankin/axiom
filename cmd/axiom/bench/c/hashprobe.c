/* Open-addressing hash table with linear probing: inserts, hits and misses. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#define CAP (1u << 20)
typedef struct { uint64_t key; uint64_t val; } Slot;
static Slot *tab;
static inline uint64_t hash(uint64_t k) { k ^= k >> 33; k *= 0xff51afd7ed558ccdULL; k ^= k >> 33; return k; }
static void put(uint64_t k, uint64_t v) {
    uint32_t i = (uint32_t)hash(k) & (CAP - 1);
    while (tab[i].key != 0 && tab[i].key != k) i = (i + 1) & (CAP - 1);
    tab[i].key = k; tab[i].val = v;
}
static int get(uint64_t k, uint64_t *v) {
    uint32_t i = (uint32_t)hash(k) & (CAP - 1);
    while (tab[i].key != 0) {
        if (tab[i].key == k) { *v = tab[i].val; return 1; }
        i = (i + 1) & (CAP - 1);
    }
    return 0;
}
int main(void) {
    tab = calloc(CAP, sizeof(Slot));
    uint64_t s = 12345, sum = 0;
    for (int i = 0; i < 700000; i++) { s = s * 6364136223846793005ULL + 1; put((s >> 16) | 1, i); }
    for (int r = 0; r < 6; r++) {
        uint64_t t = 12345;
        for (int i = 0; i < 700000; i++) {
            t = t * 6364136223846793005ULL + 1;
            uint64_t v;
            if (get((t >> 16) | 1, &v)) sum += v;
            if (get((t >> 17) | 1, &v)) sum += v ^ 1;
        }
    }
    printf("hashprobe: %llu\n", (unsigned long long)sum);
    return 0;
}
