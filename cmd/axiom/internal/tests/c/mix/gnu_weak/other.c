int overridden(void) { return 2; }
int both_weak(void) __attribute__((weak));
int both_weak(void) { return 4; }
int by_pragma(void) { return 6; }
int weak_object = 8;
