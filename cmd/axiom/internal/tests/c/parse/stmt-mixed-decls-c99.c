/* flags: --c-std=c99 */
/* Declarations after statements (C99). */
int f(int n) {
    n++;
    int a = n;
    a++;
    int b = a;
    return b;
}
