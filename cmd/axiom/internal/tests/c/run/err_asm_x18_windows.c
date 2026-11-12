/* flags: --c-std=gnu17 --target=aarch64-windows */
/* expect-error: C3096 */
/* On Windows on ARM64 x18 holds the thread's TEB (thread-locals are found
   through it): inline assembly cannot clobber it, as on macOS. */
int main(void) {
    __asm__ volatile("mov x18, #0" ::: "x18");
    return 0;
}
