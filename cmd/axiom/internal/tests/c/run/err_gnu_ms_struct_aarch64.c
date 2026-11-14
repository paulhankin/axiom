/* flags: --target=aarch64 --c-std=gnu17 */
/* check-only */
/* The attributes ms_struct and gcc_struct are x86-64's: on another target
   they are unknown (C4052, ignored) and __has_attribute says 0, and
   #pragma ms_struct is an unknown pragma. */
#if __has_attribute(ms_struct) || __has_attribute(gcc_struct)
#error "__has_attribute"
#endif
struct __attribute__((ms_struct)) A { char a : 4; short b : 4; char c; }; /* expect-warning: C4052 */
#pragma ms_struct on
struct B { char a : 4; short b : 4; char c; };
_Static_assert(sizeof(struct A) == 2 && sizeof(struct B) == 2, "A, B");
int main(void) { return 0; }
