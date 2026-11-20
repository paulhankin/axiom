/* Struct copies: small aggregates built, passed and returned by value, copied into arrays. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
typedef struct { int present; double x, y, z; } OptVec;
typedef struct { uint8_t name[25]; uint8_t tag[25]; } Pair;
typedef struct { float v[5]; } F5;
static OptVec mk(double a, double b, double c) { OptVec r; r.present = a > 0; r.x = a; r.y = b; r.z = c; return r; }
static OptVec add(OptVec a, OptVec b) { OptVec r = {a.present & b.present, a.x + b.x, a.y + b.y, a.z + b.z}; return r; }
static Pair swapped(Pair p) { Pair q; for (int i = 0; i < 25; i++) { q.name[i] = p.tag[i]; q.tag[i] = p.name[i]; } return q; }
static F5 five(float t) { F5 f; f.v[0] = t; f.v[1] = t * 2; f.v[2] = t + 1; f.v[3] = t - 1; f.v[4] = t * t; return f; }
int main(void) {
    int n = 4096;
    OptVec *ov = malloc(n * sizeof *ov);
    Pair *pv = malloc(n * sizeof *pv);
    F5 *fv = malloc(n * sizeof *fv);
    for (int i = 0; i < n; i++) { for (int k = 0; k < 25; k++) { pv[i].name[k] = (uint8_t)(i + k); pv[i].tag[k] = (uint8_t)(i * k); } }
    double sx = 0; uint64_t h = 0; float sf = 0;
    for (int r = 0; r < 4000; r++) {
        OptVec acc = mk(1, 2, 3);
        for (int i = 0; i < n; i++) {
            OptVec v = mk(i - r, i * 0.5, r * 0.25);
            acc = add(acc, v);
            ov[i] = v;
        }
        sx += acc.x + acc.y + acc.z + acc.present + ov[r].z;
        for (int i = 0; i < n; i++) pv[i] = swapped(pv[(i + r) & (n - 1)]);
        h += pv[r].name[3] + pv[n - 1 - r].tag[24];
        for (int i = 0; i < n; i++) fv[i] = five((float)(i & 255));
        for (int i = 0; i < n; i += 64) { F5 f = fv[i]; sf += f.v[0] + f.v[4]; }
    }
    printf("structcopy: %.3f %llu %.1f\n", sx, (unsigned long long)h, sf);
    return 0;
}
