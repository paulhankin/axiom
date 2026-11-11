/* Linkage: declarations of one entity in several scopes, extern in a
   block naming the file-scope object, static after a prior declaration,
   functions declared in blocks, composite types. */
#include "host.h"

static int hidden = 1;
extern int hidden;
int shared;
int shared = 10;
int arr[];
int arr[3] = { 7, 8, 9 };
static int sfunc(int);
int ext_fn(int (*)[3]);

static int sfunc(int x) { return x + hidden; }
int ext_fn(int (*p)[]) { return (*p)[2]; }

int main(void) {
    int shared = 99;
    print_i64(shared);
    {
        extern int shared;
        print_i64(shared);
        shared = 11;
    }
    print_i64(shared);
    {
        int sfunc(int);
        print_i64(sfunc(4));
    }
    {
        extern int hidden;
        hidden = 5;
    }
    print_i64(sfunc(4));
    print_i64(ext_fn(&arr));
    print_i64(sizeof arr);
    {
        extern int arr[];
        print_i64(arr[0]);
    }
    {
        extern int shared;
        print_i64(shared);
    }
    return 0;
}

/* expect: 99 */
/* expect: 10 */
/* expect: 99 */
/* expect: 5 */
/* expect: 9 */
/* expect: 9 */
/* expect: 12 */
/* expect: 7 */
/* expect: 11 */
