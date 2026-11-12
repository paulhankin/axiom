/* flags: --c-std=gnu17 */
/* native-only */
/* linux-only */
/* Raw Linux system calls from inline assembly, their arguments in explicit
   register variables (pipe2, write, read, close, getpid). */
#include "host.h"

typedef long long i64;

#ifdef __AXIOM_ASM_X86_64__
static i64 sys3(i64 n, i64 a, i64 b, i64 c) {
    register i64 rax asm("rax") = n;
    register i64 rdi asm("rdi") = a;
    register i64 rsi asm("rsi") = b;
    register i64 rdx asm("rdx") = c;
    asm volatile("syscall" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");
    return rax;
}
enum { SYS_READ = 0, SYS_WRITE = 1, SYS_CLOSE = 3, SYS_GETPID = 39, SYS_PIPE2 = 293 };
#else
static i64 sys3(i64 n, i64 a, i64 b, i64 c) {
    register i64 x8 asm("x8") = n;
    register i64 x0 asm("x0") = a;
    register i64 x1 asm("x1") = b;
    register i64 x2 asm("x2") = c;
    asm volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");
    return x0;
}
enum { SYS_READ = 63, SYS_WRITE = 64, SYS_CLOSE = 57, SYS_GETPID = 172, SYS_PIPE2 = 59 };
#endif

int main(void) {
    int fds[2];
    char buf[16];
    print_i64(sys3(SYS_PIPE2, (i64)fds, 0, 0));
    print_i64(sys3(SYS_WRITE, fds[1], (i64)"inline", 6));
    i64 n = sys3(SYS_READ, fds[0], (i64)buf, sizeof buf);
    print_i64(n);
    print_str(buf, (size_t)n);
    print_i64(sys3(SYS_CLOSE, fds[0], 0, 0) + sys3(SYS_CLOSE, fds[1], 0, 0));
    print_i64(sys3(SYS_GETPID, 0, 0, 0) > 0);
    return 0;
}
/* expect: 0 */
/* expect: 6 */
/* expect: 6 */
/* expect: inline */
/* expect: 0 */
/* expect: 1 */
