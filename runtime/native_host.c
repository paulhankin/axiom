// runtime/native_host.c - the Phalange host ABI (import module "phalange")
// for native targets, on POSIX, plus crash reporting (trap messages and stack
// traces, spec §17.4). The compiler links it into every native executable.
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <linux/futex.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/syscall.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "trap_format.h"

// Error codes shared with runtime/host.mjs and std/net.ph.
enum { PH_ERR = -1, PH_REFUSED = -2, PH_ADDR_IN_USE = -3, PH_BAD_ADDRESS = -4 };

intptr_t ph_write(int32_t fd, const uint8_t *p, size_t n) {
    ssize_t r;
    do r = write(fd, p, n);
    while (r < 0 && errno == EINTR);
    return r < 0 ? PH_ERR : (intptr_t)r;
}

intptr_t ph_read(int32_t fd, uint8_t *p, size_t n) {
    ssize_t r;
    do r = read(fd, p, n);
    while (r < 0 && errno == EINTR);
    return r < 0 ? PH_ERR : (intptr_t)r;
}

static int copy_cstr(char *dst, size_t cap, const uint8_t *s, size_t n) {
    if (n + 1 > cap) return -1;
    memcpy(dst, s, n);
    dst[n] = 0;
    return 0;
}

int32_t ph_open(const uint8_t *path, size_t len, uint32_t flags) {
    char buf[4096];
    if (copy_cstr(buf, sizeof buf, path, len)) return PH_ERR;
    int f = flags == 1 ? O_WRONLY | O_CREAT | O_TRUNC : flags == 2 ? O_WRONLY | O_CREAT | O_APPEND : O_RDONLY;
    int fd = open(buf, f | O_CLOEXEC, 0644);
    if (fd >= 0) return fd;
    return errno == ENOENT ? -2 : errno == EACCES ? -13 : PH_ERR;
}

void ph_close(int32_t fd) { close(fd); }

static int resolve(const uint8_t *name, size_t len, uint32_t port, int passive, struct addrinfo **res) {
    char host[256], serv[16];
    if (copy_cstr(host, sizeof host, name, len)) return PH_BAD_ADDRESS;
    snprintf(serv, sizeof serv, "%u", port);
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (passive) hints.ai_flags = AI_PASSIVE;
    return getaddrinfo(len ? host : NULL, serv, &hints, res) == 0 ? 0 : PH_BAD_ADDRESS;
}

int32_t ph_tcp_connect(const uint8_t *name, size_t len, uint32_t port) {
    struct addrinfo *res;
    int rc = resolve(name, len, port, 0, &res);
    if (rc) return rc;
    int result = PH_ERR;
    for (struct addrinfo *a = res; a; a = a->ai_next) {
        int fd = socket(a->ai_family, a->ai_socktype | SOCK_CLOEXEC, a->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, a->ai_addr, a->ai_addrlen) == 0) {
            result = fd;
            break;
        }
        result = errno == ECONNREFUSED ? PH_REFUSED : PH_ERR;
        close(fd);
    }
    freeaddrinfo(res);
    return result;
}

int32_t ph_tcp_listen(const uint8_t *name, size_t len, uint32_t port) {
    struct addrinfo *res;
    int rc = resolve(name, len, port, 1, &res);
    if (rc) return rc;
    int result = PH_ERR;
    for (struct addrinfo *a = res; a; a = a->ai_next) {
        int fd = socket(a->ai_family, a->ai_socktype | SOCK_CLOEXEC, a->ai_protocol);
        if (fd < 0) continue;
        int one = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
        if (bind(fd, a->ai_addr, a->ai_addrlen) == 0 && listen(fd, 128) == 0) {
            result = fd;
            break;
        }
        result = errno == EADDRINUSE ? PH_ADDR_IN_USE : PH_ERR;
        close(fd);
    }
    freeaddrinfo(res);
    return result;
}

int32_t ph_tcp_accept(int32_t listener) {
    int fd;
    do fd = accept4(listener, NULL, NULL, SOCK_CLOEXEC);
    while (fd < 0 && errno == EINTR);
    return fd < 0 ? PH_ERR : fd;
}

int32_t ph_tcp_local_port(int32_t h) {
    struct sockaddr_storage ss;
    socklen_t sl = sizeof ss;
    if (getsockname(h, (struct sockaddr *)&ss, &sl) != 0) return PH_ERR;
    if (ss.ss_family == AF_INET) return ntohs(((struct sockaddr_in *)&ss)->sin_port);
    if (ss.ss_family == AF_INET6) return ntohs(((struct sockaddr_in6 *)&ss)->sin6_port);
    return PH_ERR;
}

intptr_t ph_sock_read(int32_t h, uint8_t *p, size_t n) {
    ssize_t r;
    do r = recv(h, p, n, 0);
    while (r < 0 && errno == EINTR);
    return r < 0 ? PH_ERR : (intptr_t)r;
}

intptr_t ph_sock_write(int32_t h, const uint8_t *p, size_t n) {
    ssize_t r;
    do r = send(h, p, n, MSG_NOSIGNAL);
    while (r < 0 && errno == EINTR);
    return r < 0 ? PH_ERR : (intptr_t)r;
}

void ph_sock_close(int32_t h) { close(h); }

// ---------------------------------------------------------------------------
// Threads (std/thread) and wait/notify (@atomic_wait / @atomic_notify)
// ---------------------------------------------------------------------------
typedef void (*ph_entry)(void *);

struct thread_start {
    ph_entry entry;
    void *arg;
};

void ph_install_crash_stack(void);

static void *thread_main(void *p) {
    struct thread_start st = *(struct thread_start *)p;
    free(p);
    ph_install_crash_stack();
    st.entry(st.arg);
    return NULL;
}

// stack_top is ignored: native threads get their stack from the OS.
int32_t ph_thread_spawn(ph_entry entry, void *arg, size_t stack_top) {
    (void)stack_top;
    static int32_t next_id = 1;
    struct thread_start *st = malloc(sizeof *st);
    if (!st) return PH_ERR;
    st->entry = entry;
    st->arg = arg;
    pthread_t t;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int rc = pthread_create(&t, &attr, thread_main, st);
    pthread_attr_destroy(&attr);
    if (rc != 0) {
        free(st);
        return PH_ERR;
    }
    return __atomic_fetch_add(&next_id, 1, __ATOMIC_SEQ_CST);
}

// Native threads need no stack from the program.
size_t ph_thread_stack_size(void) { return 0; }

uint32_t ph_cpu_count(void) {
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return n > 0 ? (uint32_t)n : 1;
}

void ph_sleep_ns(uint64_t ns) {
    struct timespec ts = {(time_t)(ns / 1000000000u), (long)(ns % 1000000000u)};
    while (nanosleep(&ts, &ts) != 0 && errno == EINTR) {
    }
}

// Same results as WebAssembly memory.atomic.wait32: 0 woken, 1 not equal, 2 timed out.
int32_t ph_wait32(uint32_t *p, uint32_t expected, int64_t timeout_ns) {
    struct timespec ts, *tsp = NULL;
    if (timeout_ns >= 0) {
        ts.tv_sec = (time_t)(timeout_ns / 1000000000);
        ts.tv_nsec = (long)(timeout_ns % 1000000000);
        tsp = &ts;
    }
    for (;;) {
        long r = syscall(SYS_futex, p, FUTEX_WAIT_PRIVATE, expected, tsp, NULL, 0);
        if (r == 0) return 0;
        if (errno == EAGAIN) return 1;
        if (errno == ETIMEDOUT) return 2;
        if (errno != EINTR) return 0;
    }
}

int32_t ph_notify32(uint32_t *p, int32_t count) {
    long r = syscall(SYS_futex, p, FUTEX_WAKE_PRIVATE, count < 0 ? 0x7fffffff : count, NULL, NULL, 0);
    return r < 0 ? 0 : (int32_t)r;
}

// Defined by runtime/profile.c when profiling: writes the samples so far.
__attribute__((weak)) void ph_profile_flush(void);

// Default report for traps in native programs (test binaries override it).
__attribute__((weak)) void ph_on_trap(const struct ph_trap_site *site, const struct ph_trap_info *info,
                                      const struct ph_shadow_stack *ss) {
    static char buf[16384];
    char msg[1024];
    ph_format_trap(msg, sizeof msg, site, info);
    int n = snprintf(buf, sizeof buf, "phalange: %s at %s:%u:%u\n", msg, site->file, site->line, site->col);
    if (n < 0 || (size_t)n >= sizeof buf) n = 0;
    n += (int)ph_format_stack(buf + n, sizeof buf - (size_t)n, site, ss);
    (void)!write(2, buf, (size_t)n);
    if (ph_profile_flush) ph_profile_flush();
}

// Crashes that are not Phalange traps (a stack overflow, or a fault in C code
// called through the FFI) are reported with the shadow stack, then the
// signal is re-raised.
__attribute__((weak)) const struct ph_shadow_stack *phalange_shadow_stack(void);

static void ph_crash(int sig, siginfo_t *si, void *uc) {
    (void)uc;
    static char buf[16384];
    const char *what = sig == SIGSEGV ? "segmentation fault" : sig == SIGBUS ? "bus error" : "arithmetic fault";
    int code = sig == SIGFPE ? 3 : 5;
    uintptr_t addr = (uintptr_t)si->si_addr;
    pthread_attr_t attr;
    void *stk;
    size_t stksz;
    if (sig != SIGFPE && pthread_getattr_np(pthread_self(), &attr) == 0) {
        if (pthread_attr_getstack(&attr, &stk, &stksz) == 0 && addr + 65536 >= (uintptr_t)stk &&
            addr < (uintptr_t)stk + 65536)
            code = 8;
        pthread_attr_destroy(&attr);
    }
    int n;
    if (code == 8) n = snprintf(buf, sizeof buf, "phalange: trap T0008 (stack exhausted)\n");
    else n = snprintf(buf, sizeof buf, "phalange: crash: %s (address %p)\n", what, si->si_addr);
    if (n < 0 || (size_t)n >= sizeof buf) n = 0;
    const struct ph_shadow_stack *ss = phalange_shadow_stack ? phalange_shadow_stack() : NULL;
    n += (int)ph_format_stack(buf + n, sizeof buf - (size_t)n, NULL, ss);
    (void)!write(2, buf, (size_t)n);
    if (ph_profile_flush) ph_profile_flush();
    signal(sig, SIG_DFL);
    raise(sig);
}

// Each thread needs its own signal stack to report its stack overflow.
void ph_install_crash_stack(void) {
    stack_t st = {.ss_sp = malloc(65536), .ss_size = 65536};
    if (st.ss_sp) sigaltstack(&st, NULL);
}

__attribute__((constructor)) static void ph_install_crash_handler(void) {
    ph_install_crash_stack();
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_sigaction = ph_crash;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
}
