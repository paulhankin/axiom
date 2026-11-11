/* flags: --c-std=c11 */
/* A memory allocator over a static arena (boundary tags, a first-fit free
   list, splitting and coalescing with both neighbours), and containers
   built on it: a hash map with chaining and resizing, an AVL tree, a binary
   heap and a ring-buffer deque that grows. A pseudo-random workload drives
   them all against each other (the tree and the map must hold the same
   keys); the allocator's invariants are checked as it goes. */
#include "host.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>

/* ---- the allocator ---- */

#define ARENA_SIZE (1024 * 1024)
static alignas(16) unsigned char arena[ARENA_SIZE];

/* A block: a header (size with the low bit set when in use), the payload,
   and a footer repeating the size. Free blocks keep list links in their
   payload. Sizes are multiples of 16 and include header and footer. */
typedef struct header {
    size_t size;
    size_t pad;
} header;

typedef struct free_links {
    struct free_links *next, *prev;
} free_links;

static free_links *free_head;
static size_t in_use_bytes, peak_bytes;
static long n_alloc, n_free, n_split, n_coalesce;

#define HDR sizeof(header)
#define FTR sizeof(size_t)
#define MIN_BLOCK 48

static size_t block_size(const unsigned char *b) { return ((const header *)b)->size & ~(size_t)15; }
static bool block_used(const unsigned char *b) { return ((const header *)b)->size & 1; }
static void set_block(unsigned char *b, size_t size, bool used) {
    ((header *)b)->size = size | (used ? 1 : 0);
    *(size_t *)(b + size - FTR) = size | (used ? 1 : 0);
}
static free_links *links_of(unsigned char *b) { return (free_links *)(b + HDR); }
static unsigned char *block_of(free_links *l) { return (unsigned char *)l - HDR; }

static void list_remove(free_links *l) {
    if (l->prev) l->prev->next = l->next; else free_head = l->next;
    if (l->next) l->next->prev = l->prev;
}

static void list_push(free_links *l) {
    l->prev = NULL;
    l->next = free_head;
    if (free_head) free_head->prev = l;
    free_head = l;
}

static void heap_init(void) {
    /* sentinels: a used 16-byte block at each end so that coalescing stops */
    set_block(arena, 16 + FTR > 16 ? 32 : 16, true);
    size_t first = 32;
    size_t last = ARENA_SIZE - 32;
    set_block(arena + last, 32, true);
    set_block(arena + first, last - first, false);
    free_head = NULL;
    list_push(links_of(arena + first));
}

static void *xmalloc(size_t n) {
    size_t need = (n + HDR + FTR + 15) & ~(size_t)15;
    if (need < MIN_BLOCK) need = MIN_BLOCK;
    for (free_links *l = free_head; l; l = l->next) {
        unsigned char *b = block_of(l);
        size_t sz = block_size(b);
        if (sz < need) continue;
        list_remove(l);
        if (sz - need >= MIN_BLOCK) {
            set_block(b + need, sz - need, false);
            list_push(links_of(b + need));
            sz = need;
            n_split++;
        }
        set_block(b, sz, true);
        n_alloc++;
        in_use_bytes += sz;
        if (in_use_bytes > peak_bytes) peak_bytes = in_use_bytes;
        return b + HDR;
    }
    return NULL;
}

static void xfree(void *p) {
    if (!p) return;
    unsigned char *b = (unsigned char *)p - HDR;
    size_t sz = block_size(b);
    in_use_bytes -= sz;
    n_free++;
    /* the next block */
    unsigned char *nb = b + sz;
    if (!block_used(nb)) {
        list_remove(links_of(nb));
        sz += block_size(nb);
        n_coalesce++;
    }
    /* the previous block, by its footer */
    size_t pfoot = *(size_t *)(b - FTR);
    if (!(pfoot & 1)) {
        unsigned char *pb = b - (pfoot & ~(size_t)15);
        list_remove(links_of(pb));
        sz += block_size(pb);
        b = pb;
        n_coalesce++;
    }
    set_block(b, sz, false);
    list_push(links_of(b));
}

static void *xrealloc(void *p, size_t old, size_t n) {
    void *q = xmalloc(n);
    if (q && p) {
        unsigned char *d = q;
        const unsigned char *s = p;
        for (size_t i = 0; i < old && i < n; i++) d[i] = s[i];
    }
    xfree(p);
    return q;
}

/* walk the arena: every block's header matches its footer, no two free
   blocks are adjacent, the free list holds exactly the free blocks */
static int heap_check(void) {
    int bad = 0, nfree_walk = 0, nfree_list = 0;
    bool prev_free = false;
    for (size_t off = 32; off < ARENA_SIZE - 32;) {
        unsigned char *b = arena + off;
        size_t sz = block_size(b);
        if (sz < 16 || off + sz > ARENA_SIZE - 32) return bad + 1000;
        if (*(size_t *)(b + sz - FTR) != ((header *)b)->size) bad++;
        bool f = !block_used(b);
        if (f && prev_free) bad++;
        if (f) nfree_walk++;
        prev_free = f;
        off += sz;
    }
    for (free_links *l = free_head; l; l = l->next) nfree_list++;
    return bad + (nfree_walk != nfree_list);
}

/* ---- random numbers ---- */

static uint64_t rng_state = 0x9e3779b97f4a7c15ull;
static uint32_t rnd(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 7;
    rng_state ^= rng_state << 17;
    return (uint32_t)(rng_state >> 16);
}

/* ---- hash map: uint32 -> int64, chaining ---- */

typedef struct entry {
    uint32_t key;
    int64_t value;
    struct entry *next;
} entry;

typedef struct {
    entry **buckets;
    size_t nbuckets, count;
    int resizes;
} hmap;

static size_t hash32(uint32_t k) {
    k ^= k >> 16;
    k *= 0x7feb352du;
    k ^= k >> 15;
    k *= 0x846ca68bu;
    k ^= k >> 16;
    return k;
}

static void hm_init(hmap *m) {
    m->nbuckets = 8;
    m->count = 0;
    m->resizes = 0;
    m->buckets = xmalloc(m->nbuckets * sizeof(entry *));
    for (size_t i = 0; i < m->nbuckets; i++) m->buckets[i] = NULL;
}

static void hm_grow(hmap *m) {
    size_t nb = m->nbuckets * 2;
    entry **b = xmalloc(nb * sizeof(entry *));
    for (size_t i = 0; i < nb; i++) b[i] = NULL;
    for (size_t i = 0; i < m->nbuckets; i++) {
        entry *e = m->buckets[i];
        while (e) {
            entry *next = e->next;
            size_t h = hash32(e->key) & (nb - 1);
            e->next = b[h];
            b[h] = e;
            e = next;
        }
    }
    xfree(m->buckets);
    m->buckets = b;
    m->nbuckets = nb;
    m->resizes++;
}

static entry **hm_slot(hmap *m, uint32_t key) {
    entry **p = &m->buckets[hash32(key) & (m->nbuckets - 1)];
    while (*p && (*p)->key != key) p = &(*p)->next;
    return p;
}

static bool hm_put(hmap *m, uint32_t key, int64_t v) {
    entry **p = hm_slot(m, key);
    if (*p) { (*p)->value = v; return false; }
    entry *e = xmalloc(sizeof *e);
    e->key = key;
    e->value = v;
    e->next = NULL;
    *p = e;
    if (++m->count > m->nbuckets * 3 / 4) hm_grow(m);
    return true;
}

static bool hm_del(hmap *m, uint32_t key) {
    entry **p = hm_slot(m, key);
    if (!*p) return false;
    entry *e = *p;
    *p = e->next;
    xfree(e);
    m->count--;
    return true;
}

static const int64_t *hm_get(hmap *m, uint32_t key) {
    entry *e = *hm_slot(m, key);
    return e ? &e->value : NULL;
}

/* ---- AVL tree ---- */

typedef struct avl {
    struct avl *l, *r;
    uint32_t key;
    int height;
    int64_t value;
} avl;

static int ht(const avl *t) { return t ? t->height : 0; }
static void fix(avl *t) {
    int a = ht(t->l), b = ht(t->r);
    t->height = (a > b ? a : b) + 1;
}
static avl *rot_right(avl *t) {
    avl *l = t->l;
    t->l = l->r;
    l->r = t;
    fix(t);
    fix(l);
    return l;
}
static avl *rot_left(avl *t) {
    avl *r = t->r;
    t->r = r->l;
    r->l = t;
    fix(t);
    fix(r);
    return r;
}
static int rotations;
static avl *balance(avl *t) {
    fix(t);
    int bf = ht(t->l) - ht(t->r);
    if (bf > 1) {
        if (ht(t->l->l) < ht(t->l->r)) { t->l = rot_left(t->l); rotations++; }
        rotations++;
        return rot_right(t);
    }
    if (bf < -1) {
        if (ht(t->r->r) < ht(t->r->l)) { t->r = rot_right(t->r); rotations++; }
        rotations++;
        return rot_left(t);
    }
    return t;
}

static avl *avl_insert(avl *t, uint32_t key, int64_t v) {
    if (!t) {
        avl *n = xmalloc(sizeof *n);
        *n = (avl){ .key = key, .value = v, .height = 1 };
        return n;
    }
    if (key < t->key) t->l = avl_insert(t->l, key, v);
    else if (key > t->key) t->r = avl_insert(t->r, key, v);
    else { t->value = v; return t; }
    return balance(t);
}

static avl *avl_remove_min(avl *t, avl **min) {
    if (!t->l) { *min = t; return t->r; }
    t->l = avl_remove_min(t->l, min);
    return balance(t);
}

static avl *avl_delete(avl *t, uint32_t key) {
    if (!t) return NULL;
    if (key < t->key) t->l = avl_delete(t->l, key);
    else if (key > t->key) t->r = avl_delete(t->r, key);
    else {
        avl *l = t->l, *r = t->r;
        xfree(t);
        if (!r) return l;
        avl *m;
        r = avl_remove_min(r, &m);
        m->l = l;
        m->r = r;
        return balance(m);
    }
    return balance(t);
}

static int avl_check(const avl *t, uint32_t lo, uint32_t hi, int *count) {
    if (!t) return 0;
    (*count)++;
    int bad = 0;
    if (t->key < lo || t->key > hi) bad++;
    int bf = ht(t->l) - ht(t->r);
    if (bf < -1 || bf > 1) bad++;
    bad += avl_check(t->l, lo, t->key ? t->key - 1 : 0, count);
    bad += avl_check(t->r, t->key + 1, hi, count);
    return bad;
}

static uint64_t avl_fold(const avl *t, uint64_t h) {
    if (!t) return h;
    h = avl_fold(t->l, h);
    h = (h ^ t->key) * 0x100000001b3ull;
    h = (h ^ (uint64_t)t->value) * 0x100000001b3ull;
    return avl_fold(t->r, h);
}

static void avl_free(avl *t) {
    if (!t) return;
    avl_free(t->l);
    avl_free(t->r);
    xfree(t);
}

/* ---- binary heap of (priority, sequence) ---- */

typedef struct { uint32_t pri; uint32_t seq; } item;
typedef struct { item *a; size_t n, cap; } pheap;

static bool item_less(item x, item y) { return x.pri < y.pri || (x.pri == y.pri && x.seq < y.seq); }

static void ph_push(pheap *h, item it) {
    if (h->n == h->cap) {
        size_t nc = h->cap ? h->cap * 2 : 4;
        h->a = xrealloc(h->a, h->cap * sizeof(item), nc * sizeof(item));
        h->cap = nc;
    }
    size_t i = h->n++;
    while (i > 0 && item_less(it, h->a[(i - 1) / 2])) {
        h->a[i] = h->a[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    h->a[i] = it;
}

static item ph_pop(pheap *h) {
    item top = h->a[0];
    item last = h->a[--h->n];
    size_t i = 0;
    for (;;) {
        size_t c = 2 * i + 1;
        if (c >= h->n) break;
        if (c + 1 < h->n && item_less(h->a[c + 1], h->a[c])) c++;
        if (!item_less(h->a[c], last)) break;
        h->a[i] = h->a[c];
        i = c;
    }
    if (h->n) h->a[i] = last;
    return top;
}

/* ---- deque (ring buffer) ---- */

typedef struct { int32_t *a; size_t head, n, cap; } deque;

static void dq_grow(deque *d) {
    size_t nc = d->cap ? d->cap * 2 : 4;
    int32_t *b = xmalloc(nc * sizeof(int32_t));
    for (size_t i = 0; i < d->n; i++) b[i] = d->a[(d->head + i) % d->cap];
    xfree(d->a);
    d->a = b;
    d->head = 0;
    d->cap = nc;
}
static void dq_push_back(deque *d, int32_t v) {
    if (d->n == d->cap) dq_grow(d);
    d->a[(d->head + d->n++) % d->cap] = v;
}
static void dq_push_front(deque *d, int32_t v) {
    if (d->n == d->cap) dq_grow(d);
    d->head = (d->head + d->cap - 1) % d->cap;
    d->a[d->head] = v;
    d->n++;
}
static int32_t dq_pop_front(deque *d) {
    int32_t v = d->a[d->head];
    d->head = (d->head + 1) % d->cap;
    d->n--;
    return v;
}
static int32_t dq_pop_back(deque *d) {
    d->n--;
    return d->a[(d->head + d->n) % d->cap];
}

int main(void) {
    heap_init();
    hmap m;
    hm_init(&m);
    avl *tree = NULL;
    pheap ph = { 0 };
    deque dq = { 0 };
    uint64_t h = 0;
    int mismatches = 0, checks_bad = 0;
    int64_t heap_sum = 0, dq_sum = 0;
    uint32_t seq = 0;

    for (int round = 0; round < 20000; round++) {
        uint32_t r = rnd();
        uint32_t key = (r >> 8) % 2048;
        switch (r % 8) {
        case 0: case 1: case 2: {
            int64_t v = (int64_t)(r >> 3) - 100000;
            hm_put(&m, key, v);
            tree = avl_insert(tree, key, v);
            break;
        }
        case 3: {
            bool a = hm_del(&m, key);
            const int64_t *before = hm_get(&m, key);
            if (before) mismatches++;
            tree = avl_delete(tree, key);
            (void)a;
            break;
        }
        case 4: {
            const int64_t *v = hm_get(&m, key);
            avl *t = tree;
            while (t && t->key != key) t = key < t->key ? t->l : t->r;
            if ((v == NULL) != (t == NULL) || (v && *v != t->value)) mismatches++;
            break;
        }
        case 5:
            ph_push(&ph, (item){ .pri = r % 1000, .seq = seq++ });
            break;
        case 6:
            if (ph.n) {
                item it = ph_pop(&ph);
                heap_sum = heap_sum * 3 % 1000000007 + it.pri;
            }
            if (r & 0x100) dq_push_front(&dq, (int32_t)(r % 997)); else dq_push_back(&dq, -(int32_t)(r % 991));
            break;
        case 7:
            if (dq.n) dq_sum += (r & 0x200) ? dq_pop_front(&dq) : dq_pop_back(&dq) * 2;
            break;
        }
        if (round % 2500 == 0) checks_bad += heap_check();
    }
    int count = 0;
    checks_bad += avl_check(tree, 0, 0xffffffffu, &count);
    print_i64((int64_t)m.count);
    print_i64(count);
    print_i64(ht(tree));
    print_i64(m.resizes);
    print_i64(mismatches);
    print_i64(checks_bad);
    print_i64(heap_sum);
    print_i64(dq_sum);
    print_i64((int64_t)ph.n);
    print_i64((int64_t)dq.n);
    h = avl_fold(tree, 1469598103934665603ull);
    print_u64(h);

    /* drain the heap: must come out in order */
    item prev = { 0, 0 };
    int order_bad = 0;
    while (ph.n) {
        item it = ph_pop(&ph);
        if (item_less(it, prev)) order_bad++;
        prev = it;
    }
    print_i64(order_bad);

    /* free everything: the arena must coalesce back into one block */
    avl_free(tree);
    for (size_t i = 0; i < m.nbuckets; i++) {
        entry *e = m.buckets[i];
        while (e) {
            entry *n = e->next;
            xfree(e);
            e = n;
        }
    }
    xfree(m.buckets);
    xfree(ph.a);
    xfree(dq.a);
    int blocks = 0;
    for (free_links *l = free_head; l; l = l->next) blocks++;
    print_i64(blocks);
    print_i64((int64_t)in_use_bytes);
    print_i64(heap_check());
    print_i64(rotations);
    print_i64(n_alloc - n_free);
    print_i64(n_split > 1000 && n_coalesce > 1000);
    print_i64((int64_t)(peak_bytes / 16 * 16 == peak_bytes));
    return 0;
}

/* expect: 1483 */
/* expect: 1483 */
/* expect: 13 */
/* expect: 8 */
/* expect: 0 */
/* expect: 0 */
/* expect: 23583750 */
/* expect: -609409 */
/* expect: 47 */
/* expect: 20 */
/* expect: 391863801135857065 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */
/* expect: 2434 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
