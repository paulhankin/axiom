#pragma GCC reset_options
#pragma GCC optimize(3)
#pragma GCC reset_options
#pragma GCC optimize(1)
int a;
#pragma GCC reset_options
int b;
#pragma GCC reset_options
#pragma GCC optimize(2)
#pragma GCC reset_options
int c;
void f(int *p) {
#pragma GCC unroll 4
#pragma GCC ivdep
for (int i = 0; i < 8; i++) p[i] = 0;
#pragma GCC novector
while (*p) p++;
}
