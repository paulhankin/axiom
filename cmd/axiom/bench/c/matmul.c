/* Matrix multiply, float and double, i-k-j order, and a dot-product (i-j-k) one. */
#include <stdio.h>
#include <stdlib.h>
#define N 256
static void mm_d(int n, const double *a, const double *b, double *c) {
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++) {
            double x = a[i * n + k];
            for (int j = 0; j < n; j++) c[i * n + j] += x * b[k * n + j];
        }
}
static void mm_f(int n, const float *a, const float *b, float *c) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            float s = 0;
            for (int k = 0; k < n; k++) s += a[i * n + k] * b[j * n + k];
            c[i * n + j] = s;
        }
}
int main(void) {
    double *a = malloc(N * N * 8), *b = malloc(N * N * 8), *c = calloc(N * N, 8);
    float *fa = malloc(N * N * 4), *fb = malloc(N * N * 4), *fc = calloc(N * N, 4);
    for (int i = 0; i < N * N; i++) { a[i] = (i % 17) * 0.25; b[i] = (i % 13) * 0.5 - 1; fa[i] = (float)(i % 7); fb[i] = (float)(i % 11) * 0.125f; }
    for (int r = 0; r < 20; r++) mm_d(N, a, b, c);
    for (int r = 0; r < 20; r++) mm_f(N, fa, fb, fc);
    double s = 0;
    for (int i = 0; i < N * N; i++) s += c[i] + fc[i];
    printf("matmul: %.3f\n", s);
    return 0;
}
