// Another object of this compiler that calls the program's indirect
// function and takes its address.
int twice(int);
int ours_twice(int x) { return twice(x); }
int (*ours_twice_addr(void))(int) { return twice; }
