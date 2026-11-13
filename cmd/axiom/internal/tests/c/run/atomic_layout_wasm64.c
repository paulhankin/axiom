/* flags: --target=wasm64 --c-std=c11 */
/* check-only */
/* The layout of atomic types on wasm64 (as wasm32):
   sizes and alignments of _Atomic structures of every size class and of
   the scalar types whose layout varies, as the platform's compilers lay
   them out. */
struct s1_1 { char c[1]; };
struct s2_1 { char c[2]; };
struct s2_2 { _Alignas(2) char c[2]; };
struct s3_1 { char c[3]; };
struct s4_1 { char c[4]; };
struct s4_2 { _Alignas(2) char c[4]; };
struct s4_4 { _Alignas(4) char c[4]; };
struct s5_1 { char c[5]; };
struct s6_2 { _Alignas(2) char c[6]; };
struct s7_1 { char c[7]; };
struct s8_1 { char c[8]; };
struct s8_4 { _Alignas(4) char c[8]; };
struct s8_8 { _Alignas(8) char c[8]; };
struct s9_1 { char c[9]; };
struct s10_2 { _Alignas(2) char c[10]; };
struct s12_1 { char c[12]; };
struct s12_4 { _Alignas(4) char c[12]; };
struct s14_2 { _Alignas(2) char c[14]; };
struct s15_1 { char c[15]; };
struct s16_1 { char c[16]; };
struct s16_4 { _Alignas(4) char c[16]; };
struct s16_8 { _Alignas(8) char c[16]; };
struct s16_16 { _Alignas(16) char c[16]; };
struct s17_1 { char c[17]; };
struct s20_4 { _Alignas(4) char c[20]; };
struct s24_8 { _Alignas(8) char c[24]; };
struct s32_1 { char c[32]; };
struct s32_16 { _Alignas(16) char c[32]; };
#define L(T, s, a) _Static_assert(sizeof(_Atomic T) == (s) && _Alignof(_Atomic T) == (a), #T)
L(struct s1_1, 1, 1);
L(struct s2_1, 2, 2);
L(struct s2_2, 2, 2);
L(struct s3_1, 4, 4);
L(struct s4_1, 4, 4);
L(struct s4_2, 4, 4);
L(struct s4_4, 4, 4);
L(struct s5_1, 8, 8);
L(struct s6_2, 8, 8);
L(struct s7_1, 8, 8);
L(struct s8_1, 8, 8);
L(struct s8_4, 8, 8);
L(struct s8_8, 8, 8);
L(struct s9_1, 9, 1);
L(struct s10_2, 10, 2);
L(struct s12_1, 12, 1);
L(struct s12_4, 12, 4);
L(struct s14_2, 14, 2);
L(struct s15_1, 15, 1);
L(struct s16_1, 16, 1);
L(struct s16_4, 16, 4);
L(struct s16_8, 16, 8);
L(struct s16_16, 16, 16);
L(struct s17_1, 17, 1);
L(struct s20_4, 20, 4);
L(struct s24_8, 24, 8);
L(struct s32_1, 32, 1);
L(struct s32_16, 32, 16);
L(long double, 16, 16);
L(double _Complex, 16, 8);
L(float _Complex, 8, 8);
L(long long, 8, 8);
L(double, 8, 8);
L(long double _Complex, 32, 16);
/* an array of them: each element at the atomic size */
_Static_assert(sizeof(_Atomic struct s12_4[3]) == 3 * 12, "array");
int main(void) { return 0; }
