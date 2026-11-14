/* flags: --c-std=gnu17 */
/* expect-error: C3022 */
/* Only a comparison's lanes change signedness in an assignment: a signed
   vector that is not one is not assigned to an unsigned vector. */
typedef int s32x4 __attribute__((vector_size(16)));
typedef unsigned u32x4 __attribute__((vector_size(16)));
int main(void) {
    s32x4 a = { 1, 2, 3, 4 };
    u32x4 b = a + 1;
    return (int)b[0];
}
