/* flags: --c-std=c89 */
/* expect-error: C3031 */
/* C89 6.5.7: the items of an initializer list for an aggregate or union
   (automatic or not) are constant expressions. */
int main(void) {
    int y = 1;
    int a[2] = { y, 2 };
    return a[0];
}
