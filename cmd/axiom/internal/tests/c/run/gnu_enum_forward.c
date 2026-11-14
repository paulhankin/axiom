/* flags: --c-std=gnu11 */
/* expect-warning: C4144 */
/* A GNU dialect lets an enumeration be named before its list (6.7.2.3p3
   asks for the list first), with a warning: a forward declaration `enum e;`
   or a use, as a structure's tag may be, declares an incomplete type that
   the list completes later. Pointers to it, a function's parameters and
   return type before the list are of the same type as after it, and the
   declarations are compatible with the definitions after it. */
#include "host.h"

enum color;
struct paint { enum color *where; int (*mix)(enum color c); };
extern enum color favourite(void);
static int weight(enum color c);
enum shade *nowhere;                          /* a use declares it too */
const enum color *cview;

enum color { RED, GREEN = 5, BLUE };
enum shade { DARK = -1, LIGHT };

enum color favourite(void) { return BLUE; }
static int weight(enum color c) { return (int)c * 10; }

int main(void) {
    enum color c = GREEN;
    struct paint p = { &c, weight };
    enum color (*pick)(void) = favourite;
    cview = p.where;
    print_i64(p.mix(*p.where));
    print_i64(pick());
    print_i64(sizeof(enum color) == sizeof(unsigned int));
    print_i64(_Generic(p.where, enum color *: 1, default: 0));
    print_i64(_Generic(*cview, enum color: 1, default: 0));
    enum shade s = DARK;
    nowhere = &s;
    print_i64(*nowhere);
    print_i64((enum color)-1 > 0);            /* no negative constant: unsigned */
    return 0;
}
/* expect: 50 */
/* expect: 6 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: -1 */
/* expect: 1 */
