/* flags: --c-std=c89 */
/* Implicit int (C89, C95). */
static x;
const y = 1;
extern z;
f() {
    register r = 2;
    auto a;
    a = r;
    return a;
}
main(argc, argv)
    char **argv;
{
    return argc;
}
g(register p) {
    return p;
}
