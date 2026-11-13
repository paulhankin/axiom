/* flags: --c-std=gnu17 */
/* The functions' options and the loop pragmas in the preprocessed text:
   each change of the options as `GCC reset_options` and what gives the
   options in force from there (push_options saves them, pop_options gives
   them back), and the loop pragmas before the loop they are for. */
#pragma GCC push_options
#pragma GCC optimize ("O3", "unroll-loops")
#pragma GCC optimize 1
int a;
#pragma GCC pop_options
int b;
#pragma GCC optimize("Os")
#pragma GCC reset_options
int c;
void f(int *p) {
#pragma GCC unroll 4
#pragma GCC ivdep
    for (int i = 0; i < 8; i++) p[i] = 0;
#pragma GCC novector
    while (*p) p++;
}
