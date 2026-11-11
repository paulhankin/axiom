/* The comma operator in a for, the conditional in a constant expression. */
int a[1 ? 2 : 3];
int f(int i, int j) {
    for (i = 0, j = 1; i < j; i++, j--)
        ;
    return i, j;
}
