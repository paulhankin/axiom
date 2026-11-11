/* flags: --c-std=c89 */
/* K&R definitions: identifier lists and their declarations (until C17). */
int add(a, b)
    int a;
    int b;
{
    return a + b;
}
int scale(v, n)
    double v;
{
    return (int)v * n;
}
int empty()
{
    return 0;
}
