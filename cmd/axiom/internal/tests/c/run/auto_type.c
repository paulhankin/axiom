/* flags: --c-std=c23 */
/* C23 type inference: `auto` with no type specifier takes the type of the
   initializer after lvalue conversion (arrays and functions decay,
   qualifiers go), with the declaration's own qualifiers and storage class. */
#include "host.h"

static int twice(int x) { return 2 * x; }
static int table[4] = { 1, 2, 3, 4 };
auto file_scope = 40LL;

int main(void) {
    auto a = 3;
    auto b = 2.5f;
    auto c = "str";
    auto d = &table[1];
    auto e = table;
    auto f = twice;
    const auto g = a + 10;
    static auto h = 7LL;
    auto ch = 'c';
    unsigned char uc = 200;
    auto promoted = uc + 0;
    auto same = uc;
    print_i64(sizeof a);
    print_i64(sizeof b);
    print_i64(sizeof c == sizeof(char *));
    print_i64(d - table);
    print_i64(e[3]);
    print_i64(f(21));
    print_i64(g);
    print_i64(sizeof h);
    print_i64(sizeof ch);
    print_i64(sizeof promoted);
    print_i64(sizeof same);
    print_i64(file_scope + (long long)sizeof file_scope);
    return 0;
}
/* expect: 4 */
/* expect: 4 */
/* expect: 1 */
/* expect: 1 */
/* expect: 4 */
/* expect: 42 */
/* expect: 13 */
/* expect: 8 */
/* expect: 4 */
/* expect: 4 */
/* expect: 1 */
/* expect: 48 */
