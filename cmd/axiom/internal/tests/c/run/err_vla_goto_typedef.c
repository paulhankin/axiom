/* Nor into the scope of a typedef of a variably modified type. */
/* expect-error: C3083 */
int f(int n) {
    goto skip;
    typedef int T[n];
skip:
    return 0;
}
int main(void) { return f(1); }
