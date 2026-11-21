/* gnu89 code that uses the keywords of later standards spelled in the
   reserved namespace, as a system header reached from it does */
_Static_assert(sizeof(_Bool) == 1, "a bool is a byte");
static _Atomic int counter;
int bump(void) {
    _Bool b = 1;
    counter += b;
    return counter;
}
