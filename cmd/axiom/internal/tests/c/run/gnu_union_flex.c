/* flags: --c-std=gnu17 */
/* expect-warning: C4151 */
/* A GNU dialect takes a flexible array member as a member of a union
   (6.7.2.1p3 does not allow it), with the warning C4151: at the union's
   start, as every member, adding nothing to its size. And in a static
   initializer, a structure that is the first member of the object, a
   union, may have its flexible array member initialized too, within the
   union's size. */
#include "host.h"
union word {
    unsigned n;
    unsigned char bytes[]; /* expect-warning: C4151 */
};
struct packet {
    int kind;
    union {
        long long whole;
        short parts[]; /* expect-warning: C4151 */
    };
};
struct text { int len; char s[]; };
union holder { struct text t; char room[sizeof(struct text) + 8]; };
static union holder greeting = { { 5, "hello" } };
static union holder other = { { 2, "ok" } };
int main(void) {
    union word w = { 0x01020304u };
    struct packet p = { 7, { 0x0004000300020001LL } };
    print_i64(sizeof(union word));
    print_i64(w.bytes[0] + w.bytes[3]);
    print_i64(sizeof(struct packet) == 2 * sizeof(long long));
    print_i64(p.parts[0] + p.parts[1] + p.parts[2] + p.parts[3]);
    print_i64(sizeof greeting);
    print_str(greeting.t.s, (size_t)greeting.t.len);
    print_str(other.t.s, (size_t)other.t.len);
    print_i64(other.room[sizeof(struct text) + 2]);
    return 0;
}
/* expect: 4 */
/* expect: 5 */
/* expect: 1 */
/* expect: 10 */
/* expect: 12 */
/* expect: hello */
/* expect: ok */
/* expect: 0 */
