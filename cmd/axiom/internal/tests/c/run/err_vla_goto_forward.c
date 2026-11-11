/* A goto forward past the declaration of a VLA in the same block jumps into its scope. */
/* expect-error: C3083 */
int f(int n) {
    goto skip;
    int a[n];
skip:
    return 0;
}
int main(void) { return f(1); }
