/* A goto cannot jump into the scope of a variable length array. */
/* expect-error: C3083 */
int f(int n) {
    goto in;
    {
        int a[n];
    in:
        a[0] = 1;
        return a[0];
    }
}
int main(void) { return f(1); }
