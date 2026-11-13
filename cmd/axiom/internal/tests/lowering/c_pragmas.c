/* The pragmas for functions and loops: `GCC optimize` gives the functions
   after it their level (slow's loop stays a loop at O0: its setl; fast's is
   unrolled), `push_options` and `pop_options` take it back; `GCC target`
   their extensions (wide: AVX's vmulps, of the target's vector width); `GCC novector` keeps a
   loop scalar (one vector multiply: ivd's, and wide's), `GCC ivdep` drops
   the overlap check a vector loop has (none of cmp $0xf), `GCC unroll 0`
   keeps a loop that would be unrolled fully (its imul), `GCC unroll 8`
   unrolls by 8 (the element at 7 * 4 bytes). On AArch64 ivd and wide are
   vector loops (avx2 is not an extension there: C4112). */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | \bsetl\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | \bmulps\s+%xmm | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | \bvmulps\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | cmp\s+\$0xf, | 0
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | \bimul\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | 0x1c\(%rdi,%rax,4\) | 1
// lower: --target=aarch64 -O2 --c-freestanding --c-std=gnu17 | \bfmul\s+v\d+\.4s | 2
#pragma GCC push_options
#pragma GCC optimize ("O0")
int slow(int *a) {
    int s = 0;
    for (int i = 0; i < 4; i++)
        s += a[i];
    return s;
}
#pragma GCC pop_options
int fast(int *a) {
    int s = 0;
    for (int i = 0; i < 4; i++)
        s += a[i];
    return s;
}
void nov(float *a, const float *b, int n) {
#pragma GCC novector
    for (int i = 0; i < n; i++)
        a[i] = b[i] * 2.0f;
}
void ivd(float *a, const float *b, int n) {
#pragma GCC ivdep
    for (int i = 0; i < n; i++)
        a[i] = b[i] * 2.0f;
}
int unr(int *a) {
    int s = 0;
#pragma GCC unroll 0
    for (int i = 0; i < 4; i++)
        s += a[i] * i;
    return s;
}
int unr8(int *a) {
    int s = 0;
#pragma GCC unroll 8
    for (int i = 0; i < 64; i++)
        s ^= a[i] + i;
    return s;
}
#pragma GCC push_options
#pragma GCC target ("avx2")
void wide(float *a, const float *b, int n) {
#pragma GCC ivdep
    for (int i = 0; i < n; i++)
        a[i] = b[i] * 3.0f;
}
#pragma GCC pop_options
