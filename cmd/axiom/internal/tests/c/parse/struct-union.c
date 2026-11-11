/* Structures and unions: tags, bit-fields, nesting, forward declarations. */
struct list;
struct list {
    int v : 4, : 0, w : 3;
    unsigned : 2;
    struct list *next;
    union {
        int i;
        float f;
    } u;
} head;
union u2 { char c[4]; int i; };
struct list *tail;
