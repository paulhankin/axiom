#include <stdio.h>
int sys_fd(void) { return fileno(stdout); }
void sys_print(const char *s) { printf("%s\n", s); }
