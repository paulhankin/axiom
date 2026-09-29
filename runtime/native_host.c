// runtime/native_host.c - the Phalange host ABI (import module "phalange")
// for native targets, on POSIX, plus crash reporting (trap messages and stack
// traces, spec §16.4). The compiler links it into every native executable.
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <linux/futex.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <sys/syscall.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "trap_format.h"

// Error codes shared with runtime/host.mjs and std/net.ph.
enum { PH_ERR = -1, PH_REFUSED = -2, PH_ADDR_IN_USE = -3, PH_BAD_ADDRESS = -4, PH_TIMED_OUT = -5 };

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

// Files, directories and the process (see runtime/native.ph for the
// contract of each).
static int32_t fs_err(void) {
    switch (errno) {
    case ENOENT: return -2;
    case EACCES: case EPERM: return -13;
    case EEXIST: return -17;
    case ENOTDIR: return -20;
    case EISDIR: return -21;
    case ENOTEMPTY: return -39;
    default: return PH_ERR;
    }
}

int32_t ph_stat(const uint8_t *path, size_t len, uint32_t follow, int64_t *out) {
    char buf[4096];
    if (copy_cstr(buf, sizeof buf, path, len)) return PH_ERR;
    struct stat st;
    if ((follow ? stat(buf, &st) : lstat(buf, &st)) != 0) return fs_err();
    out[0] = S_ISREG(st.st_mode) ? 1 : S_ISDIR(st.st_mode) ? 2 : S_ISLNK(st.st_mode) ? 3 : 4;
    out[1] = (int64_t)st.st_size;
    out[2] = (int64_t)st.st_mtim.tv_sec * 1000000000 + st.st_mtim.tv_nsec;
    out[3] = st.st_mode & 07777;
    return 0;
}

int32_t ph_mkdir(const uint8_t *path, size_t len) {
    char buf[4096];
    if (copy_cstr(buf, sizeof buf, path, len)) return PH_ERR;
    return mkdir(buf, 0777) == 0 ? 0 : fs_err();
}

int32_t ph_remove(const uint8_t *path, size_t len, uint32_t dir) {
    char buf[4096];
    if (copy_cstr(buf, sizeof buf, path, len)) return PH_ERR;
    return (dir ? rmdir(buf) : unlink(buf)) == 0 ? 0 : fs_err();
}

int32_t ph_rename(const uint8_t *from, size_t flen, const uint8_t *to, size_t tlen) {
    char a[4096], b[4096];
    if (copy_cstr(a, sizeof a, from, flen) || copy_cstr(b, sizeof b, to, tlen)) return PH_ERR;
    return rename(a, b) == 0 ? 0 : fs_err();
}

intptr_t ph_readdir(const uint8_t *path, size_t len, uint8_t *buf, size_t cap) {
    char name[4096];
    if (copy_cstr(name, sizeof name, path, len)) return PH_ERR;
    DIR *d = opendir(name);
    if (!d) return fs_err();
    size_t need = 0;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
        size_t n = strlen(e->d_name);
        uint8_t kind = e->d_type == DT_REG ? 1 : e->d_type == DT_DIR ? 2 : e->d_type == DT_LNK ? 3 : e->d_type == DT_UNKNOWN ? 0 : 4;
        if (need + n + 2 <= cap) {
            buf[need] = kind;
            memcpy(buf + need + 1, e->d_name, n);
            buf[need + 1 + n] = 0;
        }
        need += n + 2;
    }
    closedir(d);
    return (intptr_t)need;
}

intptr_t ph_getcwd(uint8_t *buf, size_t cap) {
    char tmp[4096];
    if (!getcwd(tmp, sizeof tmp)) return fs_err();
    size_t n = strlen(tmp);
    if (n <= cap) memcpy(buf, tmp, n);
    return (intptr_t)n;
}

int64_t ph_seek(int32_t fd, int64_t off, uint32_t whence) {
    off_t r = lseek(fd, (off_t)off, whence == 1 ? SEEK_CUR : whence == 2 ? SEEK_END : SEEK_SET);
    return r < 0 ? fs_err() : (int64_t)r;
}

static int g_argc;
static char **g_argv;

// glibc passes main's arguments to constructors too.
__attribute__((constructor)) static void grab_args(int argc, char **argv) {
    g_argc = argc;
    g_argv = argv;
}

int32_t ph_argc(void) { return g_argc; }

intptr_t ph_arg(int32_t i, uint8_t *buf, size_t cap) {
    if (i < 0 || i >= g_argc || !g_argv) return PH_ERR;
    size_t n = strlen(g_argv[i]);
    if (n <= cap) memcpy(buf, g_argv[i], n);
    return (intptr_t)n;
}

intptr_t ph_getenv(const uint8_t *name, size_t len, uint8_t *buf, size_t cap) {
    char k[1024];
    if (copy_cstr(k, sizeof k, name, len)) return -2;
    const char *v = getenv(k);
    if (!v) return -2;
    size_t n = strlen(v);
    if (n <= cap) memcpy(buf, v, n);
    return (intptr_t)n;
}

__attribute__((weak)) void ph_profile_flush(void);

void ph_exit(int32_t code) {
    if (ph_profile_flush) ph_profile_flush();
    exit(code);
}

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
    if (r < 0) return errno == EAGAIN || errno == EWOULDBLOCK ? PH_TIMED_OUT : PH_ERR;
    return (intptr_t)r;
}

intptr_t ph_sock_write(int32_t h, const uint8_t *p, size_t n) {
    ssize_t r;
    do r = send(h, p, n, MSG_NOSIGNAL);
    while (r < 0 && errno == EINTR);
    if (r < 0) return errno == EAGAIN || errno == EWOULDBLOCK ? PH_TIMED_OUT : PH_ERR;
    return (intptr_t)r;
}

// Reads and writes on h fail with PH_TIMED_OUT after ms milliseconds (0: never).
int32_t ph_sock_timeout(int32_t h, uint32_t ms) {
    struct timeval tv = {(time_t)(ms / 1000), (suseconds_t)(ms % 1000) * 1000};
    if (setsockopt(h, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv) != 0) return PH_ERR;
    if (setsockopt(h, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv) != 0) return PH_ERR;
    return 0;
}

// Shutting the socket down first wakes a thread blocked in accept on it.
void ph_sock_close(int32_t h) {
    shutdown(h, SHUT_RDWR);
    close(h);
}

// ---------------------------------------------------------------------------
// Threads (std/thread) and wait/notify (@atomic_wait / @atomic_notify)
// ---------------------------------------------------------------------------
typedef void (*ph_entry)(void *);

struct thread_start {
    ph_entry entry;
    void *arg;
    uint32_t *exited;
};

void ph_install_crash_stack(void);

int32_t ph_notify32(uint32_t *p, int32_t count);

static void *thread_main(void *p) {
    struct thread_start st = *(struct thread_start *)p;
    free(p);
    ph_install_crash_stack();
    st.entry(st.arg);
    __atomic_store_n(st.exited, 1, __ATOMIC_SEQ_CST);
    ph_notify32(st.exited, 1);
    return NULL;
}

// stack_top is ignored: native threads get their stack from the OS.
int32_t ph_thread_spawn(ph_entry entry, void *arg, size_t stack_top, uint32_t *exited) {
    (void)stack_top;
    static int32_t next_id = 1;
    struct thread_start *st = malloc(sizeof *st);
    if (!st) return PH_ERR;
    st->entry = entry;
    st->arg = arg;
    st->exited = exited;
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

// Fills p[0..n] with cryptographically secure random bytes; 0 on success.
int32_t ph_random(uint8_t *p, size_t n) {
    size_t done = 0;
    while (done < n) {
        long r = syscall(SYS_getrandom, p + done, n - done, 0);
        if (r < 0 && errno == EINTR) continue;
        if (r <= 0) return PH_ERR;
        done += (size_t)r;
    }
    return 0;
}

// Nanoseconds on clock 0 (real time since 1970) or 1 (monotonic).
int64_t ph_clock_ns(uint32_t clock) {
    struct timespec ts;
    clock_gettime(clock == 1 ? CLOCK_MONOTONIC : CLOCK_REALTIME, &ts);
    return (int64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
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

// Linear memory (@memory_size, @memory_grow): 64 KiB pages committed from one
// region reserved at the first use. The region is as large as the system
// allows (up to 1 TiB of address space); the kernel supplies physical pages
// only when they are touched.
static uintptr_t mem_base;       // 64 KiB-aligned start of the region
static size_t mem_limit;         // its size in pages
static size_t mem_pages;         // pages handed out
static uint32_t mem_state;       // 0: not reserved, 1: reserving, 2: ready, 3: failed

static void mem_reserve(void) {
    uint32_t zero = 0;
    if (__atomic_compare_exchange_n(&mem_state, &zero, 1, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        for (size_t size = (size_t)1 << 40; size >= ((size_t)1 << 28); size /= 2) {
            void *p = mmap(NULL, size + 65536, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
            if (p != MAP_FAILED) {
                mem_base = ((uintptr_t)p + 65535) & ~(uintptr_t)65535;
                mem_limit = size / 65536;
                __atomic_store_n(&mem_state, 2, __ATOMIC_SEQ_CST);
                return;
            }
        }
        __atomic_store_n(&mem_state, 3, __ATOMIC_SEQ_CST);
        return;
    }
    while (__atomic_load_n(&mem_state, __ATOMIC_SEQ_CST) == 1) sched_yield();
}

intptr_t ph_memory_size(void) {
    if (__atomic_load_n(&mem_state, __ATOMIC_SEQ_CST) != 2) mem_reserve();
    return (intptr_t)(mem_base / 65536 + __atomic_load_n(&mem_pages, __ATOMIC_SEQ_CST));
}

// The previous size in pages (as ph_memory_size counts them), or -1.
intptr_t ph_memory_grow(intptr_t n) {
    if (__atomic_load_n(&mem_state, __ATOMIC_SEQ_CST) != 2) mem_reserve();
    if (mem_state != 2 || n < 0) return -1;
    size_t old = __atomic_load_n(&mem_pages, __ATOMIC_SEQ_CST);
    do {
        if ((size_t)n > mem_limit - old) return -1;
    } while (!__atomic_compare_exchange_n(&mem_pages, &old, old + (size_t)n, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
    return (intptr_t)(mem_base / 65536 + old);
}

// @memory_discard: the whole 64 KiB pages in the range go back to the system
// (they stay mapped, and read as zero when next touched).
void ph_memory_discard(uintptr_t addr, size_t len) {
    uintptr_t a = (addr + 65535) & ~(uintptr_t)65535, b = (addr + len) & ~(uintptr_t)65535;
    if (a < b) madvise((void *)a, b - a, MADV_DONTNEED);
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
