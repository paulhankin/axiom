/* Abstract declarators in type names. */
unsigned long n1 = sizeof(int *);
unsigned long n2 = sizeof(int[4]);
unsigned long n3 = sizeof(int (*)[4]);
unsigned long n4 = sizeof(int (*)(void));
unsigned long n5 = sizeof(int *(*)(char, ...));
unsigned long n6 = sizeof(char *const *);
