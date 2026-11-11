/* Tentative definitions (6.9.2): several in one unit are one object, an
   array's length may come later, and one never completed by an
   initializer is zero (an array without a length gets one element). */
#include "host.h"

int t1;
int t1;
int t1 = 7;
int t1;
static int t2;
static int t2;
int arr[];
int arr[3];
int lone[];
extern int ext;
int ext;
struct S;
struct S s;
struct S { int a, b; };

int main(void) {
    print_i64(t1 + t2 + ext);
    print_i64(sizeof arr / sizeof arr[0]);
    print_i64(s.a + s.b + arr[2]);
    lone[0] = 3;
    print_i64(lone[0]);
    return 0;
}
/* expect: 7 */
/* expect: 3 */
/* expect: 0 */
/* expect: 3 */
