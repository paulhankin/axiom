/* flags: --c-std=c23 */
/* A compound literal is a postfix expression: the operand of ++ and of
   sizeof, with storage-class specifiers in C23. */
int f(void) {
    int n = ++(int){1};
    unsigned long m = sizeof (static int){2} + sizeof (int){3};
    return n + (int)m;
}
