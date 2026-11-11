/* flags: --c-std=c99 */
/* C99 array parameter forms: static, qualifiers, [*]. */
void f(int a[static 3], int b[const 4], int c[const static 5], int d[static const 6]);
void g(int n, int a[*], int b[restrict], int m[n][*]);
