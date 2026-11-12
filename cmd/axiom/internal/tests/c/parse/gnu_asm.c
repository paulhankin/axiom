/* flags: --c-std=gnu17 */
/* The syntax of asm statements and file-scope asm (a GNU dialect). */
__asm__(".globl x\n");
void f(int a, int *p) {
    int r;
    asm("nop");
    __asm__ __volatile__("mov %1, %0" : "=r"(r) : "r"(a));
    asm volatile inline("" : [out] "=&r"(r), "+m"(*p) : [in] "ri"(a), "0"(1) : "memory", "cc");
    asm("" :: "r"(a));
    asm("" ::: "memory");
    asm goto("" : : "r"(a) : : l1, l2);
l1:
l2:
    asm ("a" "b" : "=r"(r));
}
