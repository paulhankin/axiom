/* Linked structures: a binary search tree built from random keys, searched, and a linked list walked. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
typedef struct Node { uint32_t key; uint32_t count; struct Node *l, *r; } Node;
typedef struct Cell { struct Cell *next; int64_t v; } Cell;
static Node *pool; static int used;
static Node *insert(Node *t, uint32_t k) {
    Node **p = &t;
    while (*p) {
        if (k == (*p)->key) { (*p)->count++; return t; }
        p = k < (*p)->key ? &(*p)->l : &(*p)->r;
    }
    Node *n = &pool[used++]; n->key = k; n->count = 1; n->l = n->r = 0; *p = n;
    return t;
}
static uint32_t find(const Node *t, uint32_t k) {
    while (t) { if (k == t->key) return t->count; t = k < t->key ? t->l : t->r; }
    return 0;
}
int main(void) {
    int n = 400000;
    pool = malloc(n * sizeof(Node));
    Node *t = 0; uint32_t s = 7;
    for (int i = 0; i < n; i++) { s = s * 1664525u + 1013904223u; t = insert(t, s >> 12); }
    uint64_t sum = 0;
    for (int r = 0; r < 4; r++) { uint32_t q = 7; for (int i = 0; i < n; i++) { q = q * 1664525u + 1013904223u; sum += find(t, (q >> 12) + r); } }
    int m = 1 << 16;
    Cell *cells = malloc(m * sizeof(Cell));
    for (int i = 0; i < m; i++) { cells[i].v = i; cells[i].next = &cells[(i * 40503u + 1) % m]; }
    Cell *c = cells; int64_t acc = 0;
    for (int i = 0; i < 20000000; i++) { acc += c->v; c = c->next; }
    printf("tree: %llu %lld\n", (unsigned long long)sum, (long long)acc);
    return 0;
}
