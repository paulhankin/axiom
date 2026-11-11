/* Declarator chains, read from the name outward. */
int *a[3];
int (*b)[3];
int **const volatile c;
int *(*d(int))(char);
void (*e[2])(void);
char (*(*f)(void))[4];
int (*g(void))(int);
