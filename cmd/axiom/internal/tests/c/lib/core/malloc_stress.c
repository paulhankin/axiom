/* The allocator through C's interface under load: many objects of random
   sizes (small, large, aligned), each filled with a pattern of its own that
   is checked before it is resized or freed, in a random order. */
#include <stdint.h>
#include <stdlib.h>
#include "print.h"

#define N 2000

static void *slot[N];
static size_t len[N];
static unsigned char tag[N];
static unsigned long long seed = 88172645463325252ULL;

static unsigned rnd(void) {
    seed ^= seed << 13;
    seed ^= seed >> 7;
    seed ^= seed << 17;
    return (unsigned)(seed >> 11);
}

static size_t size(void) {
    unsigned k = rnd() % 100;
    if (k < 60)
        return rnd() % 128;
    if (k < 90)
        return rnd() % 8192;
    if (k < 98)
        return rnd() % 200000;
    return rnd() % 2000000;
}

static void fill(int i) {
    size_t k;
    unsigned char *p = slot[i];
    for (k = 0; k < len[i]; k += 61)
        p[k] = (unsigned char)(tag[i] + k);
    if (len[i])
        p[len[i] - 1] = tag[i];
}

static int check(int i) {
    size_t k;
    unsigned char *p = slot[i];
    if (!p)
        return 1;
    for (k = 0; k < len[i]; k += 61)
        if (k != len[i] - 1 && p[k] != (unsigned char)(tag[i] + k))
            return 0;
    return len[i] == 0 || p[len[i] - 1] == tag[i];
}

int main(void) {
    long step;
    int ok = 1, i;
    for (step = 0; step < 200000 && ok; step++) {
        unsigned op = rnd() % 10;
        i = (int)(rnd() % N);
        ok &= check(i);
        if (op < 5 || !slot[i]) {
            free(slot[i]);
            len[i] = size();
            tag[i] = (unsigned char)rnd();
            if (op == 0) {
                size_t a = (size_t)1 << (rnd() % 14);
                slot[i] = aligned_alloc(a, len[i]);
                ok &= ((uintptr_t)slot[i] & (a - 1)) == 0;
            } else if (op == 1) {
                size_t k;
                unsigned char *p;
                slot[i] = calloc(len[i], 1);
                p = slot[i];
                for (k = 0; k < len[i]; k += 97)
                    ok &= p[k] == 0;
            } else {
                slot[i] = malloc(len[i]);
            }
            ok &= slot[i] != NULL;
            fill(i);
        } else if (op < 8) {
            size_t n = size(), k, keep = n < len[i] ? n : len[i];
            unsigned char *p = realloc(slot[i], n);
            ok &= p != NULL;
            /* (the kept bytes are the old ones) */
            for (k = 0; k < keep; k += 61)
                if (k != len[i] - 1)
                    ok &= p[k] == (unsigned char)(tag[i] + k);
            slot[i] = p;
            len[i] = n;
            fill(i);
        } else {
            free(slot[i]);
            slot[i] = NULL;
        }
    }
    for (i = 0; i < N; i++) {
        ok &= check(i);
        free(slot[i]);
    }
    print_bool(ok); /* expect: true */
    return 0;
}
