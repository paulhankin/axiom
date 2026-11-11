/* Nor into the scope of a pointer to a variable length array. */
/* expect-error: C3083 */
int f(int n) {
    goto skip;
    int (*p)[n];
skip:
    return 0;
}
int main(void) { return f(1); }
