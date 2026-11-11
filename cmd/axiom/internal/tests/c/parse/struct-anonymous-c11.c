/* flags: --c-std=c11 */
/* Anonymous structures and unions (C11). */
struct s {
    union {
        int a;
        float b;
    };
    struct {
        int c, d;
    };
} v;
