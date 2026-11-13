/* A C source given with --link=: compiled by this compiler for the target
   (an object in its format), with the program's C options. */
#include <stdio.h>
extern int pub_data;
int link_helper(int x) { printf("helper %d\n", x + pub_data); return x * 2; }
