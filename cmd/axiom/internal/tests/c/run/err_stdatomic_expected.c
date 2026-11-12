/* flags: --c-std=c11 */
/* expect-error: C3093 */
/* 7.17.7.4: expected is a C * */
#include <stdatomic.h>
atomic_int a;
long e;
int main(void) { return atomic_compare_exchange_strong(&a, &e, 1); }
