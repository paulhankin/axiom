/* flags: --c-std=c17 */
/* Before C23 a pointer to an array of int does not convert to a pointer to
   an array of const int (the array type itself is not qualified). */
/* expect-error: C3022 */
int main(void) {
    int x[2][3] = { 0 };
    const int (*p)[3] = x;
    return p[0][0];
}
