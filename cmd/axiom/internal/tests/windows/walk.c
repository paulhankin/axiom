/* A stack walk on Windows (run.mjs, "windows"): the system's unwinder
   (RtlCaptureStackBackTrace) goes from die through leaf and main into the
   start-up and the system's code, by the function table and the unwind
   information the compiler and linker write (at -O2 leaf and main are
   inlined into the start-up's call of main). */
unsigned short RtlCaptureStackBackTrace(unsigned long skip, unsigned long n, void **frames, unsigned long *hash);
int printf(const char *, ...);
_Noreturn void exit(int);
static void *frames[16];
static volatile int one = 1;
_Noreturn void die(int x) {
    int n = RtlCaptureStackBackTrace(0, 16, frames, 0);
    for (int i = 0; i < n; i++) {
        printf("frame %p\n", frames[i]);
    }
    exit(x);
}
int leaf(int *p) {
    if (!p) {
        die(3);
    }
    return *p;
}
int main(void) {
    printf("leaf %p main %p die %p\n", (void *)leaf, (void *)main, (void *)die);
    int *volatile q = one ? 0 : (int *)&one;
    return leaf(q);
}
