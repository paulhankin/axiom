#pragma GCC visibility push(hidden)
#pragma GCC visibility push(default)
int inner(void) { return 1; }
#pragma GCC visibility pop
int outer(void) { return 2; }
#pragma GCC visibility pop
int last(void) { return 3; }
