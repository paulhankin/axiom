/* flags: --c-std=gnu17 */
/* Vector types: an element count that is not a power of two (C3077), an
   array designator in a vector's initializer (C3072), ! of a vector
   (C3020), a cast from a vector to another size's type (C3028) and a
   __builtin_shuffle mask of the wrong type (C3082). */
/* expect-error: C3077 */
/* expect-error: C3072 */
/* expect-error: C3020 */
/* expect-error: C3028 */
/* expect-error: C3082 */
typedef int v3si __attribute__((vector_size(12)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
v4si a = {[2] = 1};
int f(v4si x) { return !x; }
double g(v4si x) { return (double)x; }
v4si h(v4si x, v4sf m) { return __builtin_shuffle(x, m); }
int main(void) { return 0; }
