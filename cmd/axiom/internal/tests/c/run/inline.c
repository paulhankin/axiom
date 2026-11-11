/* flags: --c-std=c99 */
/* inline (6.7.4): an inline definition (every file-scope declaration says
   inline, none extern) is for this unit's calls only; with an extern
   declaration (or one without inline) the definition is an external one,
   whose body may then define modifiable static objects; static inline. */
#include "host.h"

inline int twice(int x) { return 2 * x; }

inline int counter(void) {
    static int n;
    return ++n;
}
extern int counter(void);

int plain(int x);
inline int plain(int x) { return x - 1; }

static inline int square(int x) { return x * x; }

static int helper(void) { return 7; }
static inline int uses_static(void) { return helper(); }

inline int later(void) { return 9; }
int later(void);

int main(void) {
    int (*fp)(int) = twice;
    print_i64(twice(21));
    print_i64(fp(5));
    print_i64(counter() + counter());
    print_i64(plain(10));
    print_i64(square(9));
    print_i64(uses_static());
    print_i64(later());
    return 0;
}

/* expect: 42 */
/* expect: 10 */
/* expect: 3 */
/* expect: 9 */
/* expect: 81 */
/* expect: 7 */
/* expect: 9 */
