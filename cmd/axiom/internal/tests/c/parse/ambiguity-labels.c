/* A label versus an expression statement; a conditional is not a label. */
int f(int a) {
    a: a = a ? a : 1;
    b: ;
    goto a;
    goto b;
}
