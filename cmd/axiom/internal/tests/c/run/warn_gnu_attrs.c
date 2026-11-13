/* flags: --c-std=gnu11 --c-keep-null-checks */
/* expect-warning: C4052 */
/* expect-warning: C4121 */
/* Misplaced and unknown GNU attributes are ignored with a warning; a null
   pointer passed for a nonnull parameter is warned about (and, with
   --c-keep-null-checks, the test of it in the function is still made). */
static int use(int *p) __attribute__((nonnull));
static int use(int *p) { return p != 0; }
int x __attribute__((noinline));
int y __attribute__((no_such_thing));
int main(void) { return use(0); }
