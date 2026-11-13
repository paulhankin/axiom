#include <stdio.h>
int __overflow(FILE *, int);
int sys_put(int c) { return __overflow(stdout, c); }
