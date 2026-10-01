// util.h - arenas, growable arrays, string interning, source files.
#ifndef AX_UTIL_H
#define AX_UTIL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef __int128 i128;

// ---------------------------------------------------------------------------
// Arena allocator: all compiler data structures are bump-allocated and never
// freed individually.
// ---------------------------------------------------------------------------
void *arena_alloc(size_t size);  // zeroed, 16-byte aligned
char *arena_strndup(const char *s, size_t n);
char *arena_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#define NEW(T) ((T *)arena_alloc(sizeof(T)))
#define NEWN(T, n) ((T *)arena_alloc(sizeof(T) * (size_t)(n)))

// ---------------------------------------------------------------------------
// Growable arrays (heap-backed, pointer to a {data,len,cap} struct field set).
// ---------------------------------------------------------------------------
#define Vec(T) struct { T *data; int len, cap; }
#define vec_push(v, x)                                                        \
    do {                                                                      \
        if ((v).len == (v).cap) {                                             \
            (v).cap = (v).cap ? (v).cap * 2 : 8;                              \
            (v).data = realloc((v).data, sizeof(*(v).data) * (size_t)(v).cap); \
        }                                                                     \
        (v).data[(v).len++] = (x);                                            \
    } while (0)
#define vec_pop(v) ((v).data[--(v).len])
#define vec_last(v) ((v).data[(v).len - 1])

// ---------------------------------------------------------------------------
// Interned strings: identical contents => identical pointer.
// ---------------------------------------------------------------------------
typedef const char *Sym;
Sym intern(const char *s, size_t n);
Sym intern_cstr(const char *s);

// Sym -> pointer hash map.
typedef struct SymMap {
    Sym *keys;
    void **vals;
    int cap, len;
} SymMap;
void *symmap_get(SymMap *m, Sym k);
void symmap_put(SymMap *m, Sym k, void *v);

typedef Vec(char) CharVec;
typedef Vec(Sym) SymVec;

// ---------------------------------------------------------------------------
// Source files and spans.
// ---------------------------------------------------------------------------
typedef struct SourceFile {
    const char *path;     // as given / resolved
    const char *display;  // path shown in diagnostics
    const char *text;
    size_t len;
    int id;
    uint32_t *line_starts;  // built on first use
    int nlines;
} SourceFile;

typedef struct Span {
    int file;
    uint32_t start, end;  // byte offsets, end exclusive
} Span;

typedef Vec(SourceFile *) FileVec;
extern FileVec g_files;

SourceFile *load_file(const char *path, const char *display);
void span_linecol(Span s, int *sl, int *sc, int *el, int *ec);
Span span_join(Span a, Span b);

char *read_entire_file(const char *path, size_t *len);

#endif
