// A weak definition (another unit's strong one wins), a weak reference
// nothing defines, a thread-local, a constructor, data holding an address.
__attribute__((weak)) int wf(void) { return 1; }
__attribute__((weak)) int wd = 7;
extern __attribute__((weak)) int wu(void);
__thread int tl = 3;
int gettl(void) { return tl++; }
__attribute__((constructor)) static void init(void) { wd++; }
int callwu(void) { return wu ? wu() : 0; }
int (*fp)(void) = wf;
extern int ext;
int useext(void) { return ext; }
