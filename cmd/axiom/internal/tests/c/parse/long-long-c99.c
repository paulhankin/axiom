/* flags: --c-std=c99 */
/* long long and the C99 keywords. */
long long a;
unsigned long long int b;
_Bool c;
inline int f(int *restrict p) {
    return *p;
}
