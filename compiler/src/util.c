// util.c - arenas, interning, source files.
#include "util.h"

#include <stdarg.h>

// ---------------------------------------------------------------------------
// Arena
// ---------------------------------------------------------------------------
#define ARENA_BLOCK (1u << 20)

static char *arena_cur, *arena_end;

void *arena_alloc(size_t size) {
    size = (size + 15) & ~(size_t)15;
    if ((size_t)(arena_end - arena_cur) < size) {
        size_t bs = size > ARENA_BLOCK ? size : ARENA_BLOCK;
        arena_cur = calloc(1, bs);
        if (!arena_cur) {
            fprintf(stderr, "phalange: out of memory\n");
            exit(2);
        }
        arena_end = arena_cur + bs;
    }
    void *p = arena_cur;
    arena_cur += size;
    return p;
}

char *arena_strndup(const char *s, size_t n) {
    char *p = arena_alloc(n + 1);
    memcpy(p, s, n);
    p[n] = 0;
    return p;
}

char *arena_printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buf[512];
    int n = vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    if (n < (int)sizeof buf) return arena_strndup(buf, (size_t)n);
    char *p = arena_alloc((size_t)n + 1);
    va_start(ap, fmt);
    vsnprintf(p, (size_t)n + 1, fmt, ap);
    va_end(ap);
    return p;
}

// ---------------------------------------------------------------------------
// Interning (open addressing hash table)
// ---------------------------------------------------------------------------
typedef struct {
    const char *str;
    size_t len;
    uint64_t hash;
} InternEntry;

static InternEntry *itab;
static size_t itab_cap, itab_len;

static uint64_t hash_bytes(const char *s, size_t n) {
    uint64_t h = 1469598103934665603ull;
    for (size_t i = 0; i < n; i++) h = (h ^ (unsigned char)s[i]) * 1099511628211ull;
    return h;
}

Sym intern(const char *s, size_t n) {
    if (itab_len * 2 >= itab_cap) {
        size_t ncap = itab_cap ? itab_cap * 2 : 4096;
        InternEntry *nt = calloc(ncap, sizeof *nt);
        for (size_t i = 0; i < itab_cap; i++) {
            if (!itab[i].str) continue;
            size_t j = itab[i].hash & (ncap - 1);
            while (nt[j].str) j = (j + 1) & (ncap - 1);
            nt[j] = itab[i];
        }
        free(itab);
        itab = nt;
        itab_cap = ncap;
    }
    uint64_t h = hash_bytes(s, n);
    size_t j = h & (itab_cap - 1);
    while (itab[j].str) {
        if (itab[j].hash == h && itab[j].len == n && memcmp(itab[j].str, s, n) == 0) return itab[j].str;
        j = (j + 1) & (itab_cap - 1);
    }
    itab[j].str = arena_strndup(s, n);
    itab[j].len = n;
    itab[j].hash = h;
    itab_len++;
    return itab[j].str;
}

Sym intern_cstr(const char *s) { return intern(s, strlen(s)); }

// ---------------------------------------------------------------------------
// SymMap
// ---------------------------------------------------------------------------
static size_t ptr_hash(const void *p) {
    uint64_t x = (uint64_t)(uintptr_t)p;
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdull;
    x ^= x >> 33;
    return (size_t)x;
}

void *symmap_get(SymMap *m, Sym k) {
    if (!m->cap) return NULL;
    size_t j = ptr_hash(k) & (size_t)(m->cap - 1);
    while (m->keys[j]) {
        if (m->keys[j] == k) return m->vals[j];
        j = (j + 1) & (size_t)(m->cap - 1);
    }
    return NULL;
}

void symmap_put(SymMap *m, Sym k, void *v) {
    if ((m->len + 1) * 2 >= m->cap) {
        int ncap = m->cap ? m->cap * 2 : 16;
        Sym *nk = calloc((size_t)ncap, sizeof *nk);
        void **nv = calloc((size_t)ncap, sizeof *nv);
        for (int i = 0; i < m->cap; i++) {
            if (!m->keys[i]) continue;
            size_t j = ptr_hash(m->keys[i]) & (size_t)(ncap - 1);
            while (nk[j]) j = (j + 1) & (size_t)(ncap - 1);
            nk[j] = m->keys[i];
            nv[j] = m->vals[i];
        }
        free(m->keys);
        free(m->vals);
        m->keys = nk;
        m->vals = nv;
        m->cap = ncap;
    }
    size_t j = ptr_hash(k) & (size_t)(m->cap - 1);
    while (m->keys[j]) {
        if (m->keys[j] == k) {
            m->vals[j] = v;
            return;
        }
        j = (j + 1) & (size_t)(m->cap - 1);
    }
    m->keys[j] = k;
    m->vals[j] = v;
    m->len++;
}

// ---------------------------------------------------------------------------
// Files
// ---------------------------------------------------------------------------
FileVec g_files;

char *read_entire_file(const char *path, size_t *len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = arena_alloc((size_t)n + 1);
    if (n > 0 && fread(buf, 1, (size_t)n, f) != (size_t)n) {
        fclose(f);
        return NULL;
    }
    buf[n] = 0;
    fclose(f);
    *len = (size_t)n;
    return buf;
}

SourceFile *load_file(const char *path, const char *display) {
    size_t len;
    char *text = read_entire_file(path, &len);
    if (!text) return NULL;
    SourceFile *sf = NEW(SourceFile);
    sf->path = arena_strndup(path, strlen(path));
    sf->display = display ? display : sf->path;
    sf->text = text;
    sf->len = len;
    sf->id = g_files.len;
    vec_push(g_files, sf);
    return sf;
}

static void offset_linecol(SourceFile *f, uint32_t off, int *line, int *col) {
    if (!f->line_starts) {
        int n = 1;
        for (size_t i = 0; i < f->len; i++)
            if (f->text[i] == '\n') n++;
        f->line_starts = NEWN(uint32_t, n);
        f->line_starts[0] = 0;
        int k = 1;
        for (size_t i = 0; i < f->len; i++)
            if (f->text[i] == '\n') f->line_starts[k++] = (uint32_t)i + 1;
        f->nlines = n;
    }
    if (off > f->len) off = (uint32_t)f->len;
    int lo = 0, hi = f->nlines - 1;  // last line whose start <= off
    while (lo < hi) {
        int mid = (lo + hi + 1) / 2;
        if (f->line_starts[mid] <= off) lo = mid;
        else hi = mid - 1;
    }
    int c = 1;
    for (uint32_t i = f->line_starts[lo]; i < off; i++)
        if (((unsigned char)f->text[i] & 0xC0) != 0x80) c++;  // count Unicode scalar values
    *line = lo + 1;
    *col = c;
}

void span_linecol(Span s, int *sl, int *sc, int *el, int *ec) {
    SourceFile *f = g_files.data[s.file];
    offset_linecol(f, s.start, sl, sc);
    offset_linecol(f, s.end, el, ec);
}

Span span_join(Span a, Span b) {
    Span r = a;
    if (b.file == a.file) {
        if (b.start < r.start) r.start = b.start;
        if (b.end > r.end) r.end = b.end;
    }
    return r;
}
