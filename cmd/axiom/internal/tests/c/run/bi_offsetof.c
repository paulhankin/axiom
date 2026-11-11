/* offsetof (<stddef.h>): members, nested members and subscripts, members
   of anonymous structures, unions, an integer constant expression usable as
   an array size and in a case label. */
#include <stddef.h>
#include "host.h"

struct in { char c; double d[3]; };
struct s {
    char a;
    int b[4];
    struct in in[2];
    union { short u; long long v; } un;
};
typedef struct s S;
union w { char c; int i[5]; };

static char sized[offsetof(struct s, b[3]) + 1];

int main(void) {
    int k = (int)offsetof(S, b[1]);
    print_i64(offsetof(struct s, a));
    print_i64(offsetof(struct s, b) == sizeof(int));
    print_i64(offsetof(S, b[2]) - offsetof(S, b));
    print_i64(offsetof(S, in[1].d[2]) - offsetof(S, in) == sizeof(struct in) + offsetof(struct in, d) + 2 * sizeof(double));
    print_i64(offsetof(union w, i[4]));
    print_i64(offsetof(S, un.v) == offsetof(S, un));
    print_i64(sizeof sized == offsetof(struct s, b) + 3 * sizeof(int) + 1);
    switch (k) {
    case offsetof(S, b[1]): print_i64(1); break;
    default: print_i64(0);
    }
    return 0;
}

/* expect: 0 */
/* expect: 1 */
/* expect: 8 */
/* expect: 1 */
/* expect: 16 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
