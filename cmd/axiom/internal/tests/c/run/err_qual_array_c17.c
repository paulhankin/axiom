/* expect-error: C3020 */
/* Before C23, pointers to arrays whose elements differ in qualifiers point
   to incompatible types (C17 6.7.3p10): they are no operands of ?:. */
int main(void) {
    int x[2][3] = { { 0 } };
    const int (*cp)[3] = (const int (*)[3])x;
    int c = 1;
    return (c ? x : cp) != 0;
}
