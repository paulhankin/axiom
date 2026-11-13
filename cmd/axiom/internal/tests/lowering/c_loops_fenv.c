/* A loop of a function with FENV_ACCESS ON (strict floating point) keeps
   its floating-point operations scalar, as written: no vector loop
   (vectorize.ax, and idioms.pure: a pinned operation is not pure); the
   same loop without the pragma is vectorized (c_loops.c). */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bmulps\b | 0
// lower: --target=aarch64 -O2 --c-freestanding | \bfmul\s+v\d+\.4s | 0
#pragma STDC FENV_ACCESS ON
void saxpy(float *restrict y, const float *restrict x, float a, int n) {
    for (int i = 0; i < n; i++) {
        y[i] = a * x[i] + y[i];
    }
}
