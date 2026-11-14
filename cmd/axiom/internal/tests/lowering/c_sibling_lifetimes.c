/* A call in tail position after the end of the block whose object's
   address it passed earlier is a sibling call (a jump: the object is no
   longer alive, 6.2.4p6; tailrec.ax mark_frame_dead), at -O2; while such an
   object is alive (the function's body's, or the call in its block) the
   call keeps the frame. */
// lower-selfhost: --target=native -O2 --c-freestanding | leave\n\s+\w+:\s+jmp\s | 1
// lower-selfhost: --target=native -O2 --c-freestanding | \bcall\s | 5
// lower-selfhost: --target=native -O1 --c-freestanding | leave\n\s+\w+:\s+jmp\s | 0
// lower: --target=aarch64 -O2 --c-freestanding | \sb\s+\w+ <bt> | 1
// lower: --target=aarch64 -O2 --c-freestanding | \sbl\s+\w+ <bt> | 5
struct cont { int node; const struct cont *next; };
int bt(int n, const char *s, const struct cont *k);

int star(int n, const char *s, const struct cont *k, const char *last) {
    if (s != last) {
        struct cont again = { -(n + 2), k };
        if (bt(n + 1, s, &again)) return 1;
    }
    return bt(k->node, s, k->next);
}

int alive_body(int n, const char *s, const struct cont *k) {
    struct cont c = { n, k };
    if (bt(n + 1, s, &c)) return 1;
    return bt(k->node, s, k->next);
}

int alive_block(int n, const char *s, const struct cont *k) {
    if (n > 0) {
        struct cont c = { n, k };
        if (bt(n + 1, s, &c)) return 1;
        return bt(k->node, s, k->next);
    }
    return 0;
}
