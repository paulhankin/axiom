#include <stdio.h>
#include <string.h>
static int counter = 5;
__thread int tlsv = 7;
int sys_add(int a, int b) { return a + b + counter; }
void sys_print(const char *s) { printf("from the system's compiler: %s %zu\n", s, strlen(s)); }
int sys_tls(void) { return ++tlsv; }
__attribute__((constructor)) static void init(void) { counter = 10; }
