// std/internal/runtime/native_host.c - the Axiom host ABI (import module "axiom")
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

// Error codes shared with std/internal/runtime/host.mjs and std/net.ax.
enum { AX_ERR = -1, AX_REFUSED = -2, AX_ADDR_IN_USE = -3, AX_BAD_ADDRESS = -4, AX_TIMED_OUT = -5 };

intptr_t ax_write(int32_t fd, const uint8_t *p, size_t n) {
    ssize_t r;
    do r = write(fd, p, n);
    while (r < 0 && errno == EINTR);
    return r < 0 ? AX_ERR : (intptr_t)r;
}

intptr_t ax_read(int32_t fd, uint8_t *p, size_t n) {
    ssize_t r;
    do r = read(fd, p, n);
    while (r < 0 && errno == EINTR);
    return r < 0 ? AX_ERR : (intptr_t)r;
}

static int copy_cstr(char *dst, size_t cap, const uint8_t *s, size_t n) {
    if (n + 1 > cap) return -1;
    if (memchr(s, 0, n)) return -1;  /* a NUL would silently cut the name short */
    memcpy(dst, s, n);
    dst[n] = 0;
    return 0;
}

int32_t ax_open(const uint8_t *path, size_t len, uint32_t flags) {
    char buf[4096];
    if (copy_cstr(buf, sizeof buf, path, len)) return AX_ERR;
    int f = flags == 1 ? O_WRONLY | O_CREAT | O_TRUNC : flags == 2 ? O_WRONLY | O_CREAT | O_APPEND : flags == 3 ? O_RDWR : O_RDONLY;
    int fd = open(buf, f | O_CLOEXEC, 0644);
    if (fd >= 0) return fd;
    return errno == ENOENT ? -2 : errno == EACCES ? -13 : AX_ERR;
}

void ax_close(int32_t fd) { close(fd); }

// Files, directories and the process (see std/internal/runtime/native.ax for the
// contract of each).
static int32_t fs_err(void) {
    switch (errno) {
    case ENOENT: return -2;
    case EACCES: case EPERM: return -13;
    case EEXIST: return -17;
    case ENOTDIR: return -20;
    case EISDIR: return -21;
    case ENOTEMPTY: return -39;
    case ELOOP: return -40;
    default: return AX_ERR;
    }
}

int32_t ax_create(const uint8_t *path, size_t len, uint32_t mode) {
    char buf[4096];
    if (copy_cstr(buf, sizeof buf, path, len)) return AX_ERR;
    int fd = open(buf, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, (mode_t)(mode & 0777));
    return fd >= 0 ? fd : fs_err();
}

static void stat_out(const struct stat *st, int64_t *out) {
    out[0] = S_ISREG(st->st_mode) ? 1 : S_ISDIR(st->st_mode) ? 2 : S_ISLNK(st->st_mode) ? 3 : 4;
    out[1] = (int64_t)st->st_size;
    /* (nanoseconds overflow 64 bits for times past 2262: clamp) */
    int64_t sec = (int64_t)st->st_mtim.tv_sec;
    if (sec > 9223372035) sec = 9223372035;
    if (sec < -9223372035) sec = -9223372035;
    out[2] = sec * 1000000000 + st->st_mtim.tv_nsec;
    out[3] = st->st_mode & 07777;
}

int32_t ax_stat(const uint8_t *path, size_t len, uint32_t follow, int64_t *out) {
    char buf[4096];
    if (copy_cstr(buf, sizeof buf, path, len)) return AX_ERR;
    struct stat st;
    if ((follow ? stat(buf, &st) : lstat(buf, &st)) != 0) return fs_err();
    stat_out(&st, out);
    return 0;
}

int32_t ax_fstat(int32_t fd, int64_t *out) {
    struct stat st;
    if (fstat(fd, &st) != 0) return fs_err();
    stat_out(&st, out);
    return 0;
}

int32_t ax_mkdir(const uint8_t *path, size_t len, uint32_t mode) {
    char buf[4096];
    if (copy_cstr(buf, sizeof buf, path, len)) return AX_ERR;
    return mkdir(buf, (mode_t)(mode & 07777)) == 0 ? 0 : fs_err();
}

int32_t ax_remove(const uint8_t *path, size_t len, uint32_t dir) {
    char buf[4096];
    if (copy_cstr(buf, sizeof buf, path, len)) return AX_ERR;
    return (dir ? rmdir(buf) : unlink(buf)) == 0 ? 0 : fs_err();
}

int32_t ax_rename(const uint8_t *from, size_t flen, const uint8_t *to, size_t tlen) {
    char a[4096], b[4096];
    if (copy_cstr(a, sizeof a, from, flen) || copy_cstr(b, sizeof b, to, tlen)) return AX_ERR;
    return rename(a, b) == 0 ? 0 : fs_err();
}

static intptr_t read_entries(DIR *d, uint8_t *buf, size_t cap) {
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
    return (intptr_t)need;
}

intptr_t ax_readdir(const uint8_t *path, size_t len, uint8_t *buf, size_t cap) {
    char name[4096];
    if (copy_cstr(name, sizeof name, path, len)) return AX_ERR;
    DIR *d = opendir(name);
    if (!d) return fs_err();
    intptr_t r = read_entries(d, buf, cap);
    closedir(d);
    return r;
}

int32_t ax_dir_open(int32_t dir, const uint8_t *name, size_t len) {
    char buf[4096];
    if (copy_cstr(buf, sizeof buf, name, len)) return AX_ERR;
    int fd = openat(dir < 0 ? AT_FDCWD : dir, buf, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    return fd < 0 ? fs_err() : fd;
}

intptr_t ax_dir_read(int32_t dir, uint8_t *buf, size_t cap) {
    int fd = dup(dir);
    if (fd < 0) return fs_err();
    DIR *d = fdopendir(fd);
    if (!d) {
        close(fd);
        return fs_err();
    }
    rewinddir(d);
    intptr_t r = read_entries(d, buf, cap);
    closedir(d);
    return r;
}

int32_t ax_dir_remove(int32_t dir, const uint8_t *name, size_t len, uint32_t isdir) {
    char buf[4096];
    if (copy_cstr(buf, sizeof buf, name, len)) return AX_ERR;
    return unlinkat(dir < 0 ? AT_FDCWD : dir, buf, isdir ? AT_REMOVEDIR : 0) == 0 ? 0 : fs_err();
}

void ax_dir_close(int32_t dir) { close(dir); }

/* out: device, inode -- the identity of an open directory (to check one
   reopened through a child's "..") */
int32_t ax_dir_stat(int32_t dir, int64_t *out) {
    struct stat st;
    if (fstat(dir, &st) != 0) return fs_err();
    out[0] = (int64_t)st.st_dev;
    out[1] = (int64_t)st.st_ino;
    return 0;
}

// The running program's own file. Its length.
intptr_t ax_self_path(uint8_t *buf, size_t cap) {
    char tmp[4096];
    ssize_t n = readlink("/proc/self/exe", tmp, sizeof tmp);
    if (n < 0) return fs_err();
    if ((size_t)n <= cap) memcpy(buf, tmp, (size_t)n);
    return (intptr_t)n;
}

intptr_t ax_getcwd(uint8_t *buf, size_t cap) {
    char tmp[4096];
    if (!getcwd(tmp, sizeof tmp)) return fs_err();
    size_t n = strlen(tmp);
    if (n <= cap) memcpy(buf, tmp, n);
    return (intptr_t)n;
}

int64_t ax_seek(int32_t fd, int64_t off, uint32_t whence) {
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

int32_t ax_argc(void) { return g_argc; }

intptr_t ax_arg(int32_t i, uint8_t *buf, size_t cap) {
    if (i < 0 || i >= g_argc || !g_argv) return AX_ERR;
    size_t n = strlen(g_argv[i]);
    if (n <= cap) memcpy(buf, g_argv[i], n);
    return (intptr_t)n;
}

intptr_t ax_getenv(const uint8_t *name, size_t len, uint8_t *buf, size_t cap) {
    char k[1024];
    if (copy_cstr(k, sizeof k, name, len)) return -2;
    const char *v = getenv(k);
    if (!v) return -2;
    size_t n = strlen(v);
    if (n <= cap) memcpy(buf, v, n);
    return (intptr_t)n;
}

__attribute__((weak)) void ax_profile_flush(void);

void ax_exit(int32_t code) {
    if (ax_profile_flush) ax_profile_flush();
    exit(code);
}

// -----------------------------------------------------------------------------
// Processes, the environment and signals (see std/internal/runtime/native.ax for the
// contract of each)
// -----------------------------------------------------------------------------

#include <spawn.h>
#include <sys/wait.h>

extern char **environ;

int32_t ax_getpid(void) { return (int32_t)getpid(); }

int32_t ax_fork(void) {
    pid_t pid = fork();
    return pid < 0 ? AX_ERR : (int32_t)pid;
}

int32_t ax_chdir(const uint8_t *path, size_t len) {
    char buf[4096];
    if (copy_cstr(buf, sizeof buf, path, len)) return AX_ERR;
    return chdir(buf) == 0 ? 0 : fs_err();
}

static int env_name_ok(const uint8_t *k, size_t n) {
    if (n == 0) return 0;
    for (size_t i = 0; i < n; i++)
        if (k[i] == '=' || k[i] == 0) return 0;
    return 1;
}

int32_t ax_setenv(const uint8_t *k, size_t kn, const uint8_t *v, size_t vn) {
    if (!env_name_ok(k, kn) || memchr(v, 0, vn)) return AX_ERR;
    char *name = strndup((const char *)k, kn), *value = strndup((const char *)v, vn);
    // unsetenv removes every entry for the name (the initial environment can
    // hold several), setenv only the first
    int r = name && value && unsetenv(name) == 0 ? setenv(name, value, 1) : -1;
    free(name);
    free(value);
    return r == 0 ? 0 : AX_ERR;
}

int32_t ax_unsetenv(const uint8_t *k, size_t kn) {
    if (!env_name_ok(k, kn)) return AX_ERR;
    char *name = strndup((const char *)k, kn);
    int r = name ? unsetenv(name) : -1;
    free(name);
    return r == 0 ? 0 : AX_ERR;
}

intptr_t ax_environ(uint8_t *buf, size_t cap) {
    size_t n = 0;
    for (char **e = environ; e && *e; e++) {
        size_t l = strlen(*e) + 1;
        if (n + l <= cap) memcpy(buf + n, *e, l);
        n += l;
    }
    return (intptr_t)n;
}

// Writing to a pipe whose reader has gone should fail, not kill the program.
static uint8_t sig_actions[65];
static void ignore_sigpipe(void) {
    static int done;
    if (!__atomic_exchange_n(&done, 1, __ATOMIC_SEQ_CST) && sig_actions[SIGPIPE] != 2) signal(SIGPIPE, SIG_IGN);
}

int32_t ax_proc_spawn(const uint8_t *spec, size_t len, int32_t *stdio) {
    // argv, then the environment, each a list of 0-terminated strings ending
    // in an empty one; then the directory
    size_t i = 0, argc = 0, envc = 0;
    while (i < len && spec[i]) { i += strnlen((const char *)spec + i, len - i) + 1; argc++; }
    i++;
    size_t env_at = i;
    while (i < len && spec[i]) { i += strnlen((const char *)spec + i, len - i) + 1; envc++; }
    i++;
    if (argc == 0 || i > len) return AX_ERR;
    char **argv = calloc(argc + 1, sizeof *argv), **envp = calloc(envc + 1, sizeof *envp);
    char *cwd = strndup((const char *)spec + i, len - i);
    const char *path_var = NULL;
    int32_t result = AX_ERR;
    int fds[6] = {-1, -1, -1, -1, -1, -1};
    posix_spawn_file_actions_t fa;
    posix_spawnattr_t attr;
    posix_spawn_file_actions_init(&fa);
    posix_spawnattr_init(&attr);
    if (!argv || !envp || !cwd) goto out;
    i = 0;
    for (size_t k = 0; k < argc; k++) { argv[k] = (char *)spec + i; i += strlen(argv[k]) + 1; }
    i = env_at;
    for (size_t k = 0; k < envc; k++) {
        envp[k] = (char *)spec + i;
        if (!strncmp(envp[k], "PATH=", 5)) path_var = envp[k] + 5;
        i += strlen(envp[k]) + 1;
    }
    for (int k = 0; k < 3; k++) {
        if (stdio[k] == 1) {
            int p[2];
            if (pipe2(p, O_CLOEXEC)) goto out;
            fds[2 * k] = k == 0 ? p[1] : p[0];
            fds[2 * k + 1] = k == 0 ? p[0] : p[1];
            posix_spawn_file_actions_adddup2(&fa, fds[2 * k + 1], k);
        } else if (stdio[k] == 2) {
            posix_spawn_file_actions_addopen(&fa, k, "/dev/null", O_RDWR, 0);
        }
    }
    if (*cwd) posix_spawn_file_actions_addchdir_np(&fa, cwd);
    sigset_t def;
    sigemptyset(&def);
    sigaddset(&def, SIGPIPE);
    posix_spawnattr_setsigdefault(&attr, &def);
    posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGDEF);
    pid_t pid = -1;
    int err = ENOENT;
    if (strchr(argv[0], '/')) {
        err = posix_spawn(&pid, argv[0], &fa, &attr, argv, envp);
    } else {
        // each directory of the child's PATH in turn
        const char *pv = path_var && *path_var ? path_var : "/usr/local/bin:/usr/bin:/bin";
        char cand[4096];
        for (const char *a = pv;; ) {
            const char *b = strchr(a, ':');
            size_t dl = b ? (size_t)(b - a) : strlen(a);
            int n = snprintf(cand, sizeof cand, "%.*s/%s", (int)(dl ? dl : 1), dl ? a : ".", argv[0]);
            if (n > 0 && (size_t)n < sizeof cand) {
                int e = posix_spawn(&pid, cand, &fa, &attr, argv, envp);
                if (e == 0) { err = 0; break; }
                if (e != ENOENT && e != ENOTDIR) err = e;
            }
            if (!b) break;
            a = b + 1;
        }
    }
    if (err == 0) result = (int32_t)pid;
    else result = err == ENOENT ? -2 : (err == EACCES || err == EPERM) ? -13 : AX_ERR;
out:
    for (int k = 0; k < 3; k++) {
        if (fds[2 * k + 1] >= 0) close(fds[2 * k + 1]);
        if (result >= 0 && stdio[k] == 1) {
            stdio[k] = fds[2 * k];
        } else {
            if (fds[2 * k] >= 0) close(fds[2 * k]);
            stdio[k] = -1;
        }
    }
    posix_spawn_file_actions_destroy(&fa);
    posix_spawnattr_destroy(&attr);
    free(argv);
    free(envp);
    free(cwd);
    return result;
}

int32_t ax_proc_wait(int32_t h, uint32_t block, int32_t *status) {
    int st = 0;
    pid_t r;
    do r = waitpid(h, &st, block ? 0 : WNOHANG);
    while (r < 0 && errno == EINTR);
    if (r < 0) return AX_ERR;
    if (r == 0) return 0;
    if (WIFEXITED(st)) { *status = WEXITSTATUS(st); return 1; }
    *status = WTERMSIG(st);
    return 2;
}

int32_t ax_proc_kill(int32_t h, int32_t sig) { return kill(h, sig) == 0 ? 0 : AX_ERR; }

intptr_t ax_pipe_read(int32_t h, uint8_t *p, size_t n) {
    ssize_t r;
    do r = read(h, p, n);
    while (r < 0 && errno == EINTR);
    return r < 0 ? AX_ERR : (intptr_t)r;
}

intptr_t ax_pipe_write(int32_t h, const uint8_t *p, size_t n) {
    ignore_sigpipe();
    ssize_t r;
    do r = write(h, p, n);
    while (r < 0 && errno == EINTR);
    return r < 0 ? AX_ERR : (intptr_t)r;
}

void ax_pipe_close(int32_t h) { close(h); }

static uint64_t sig_pending;
static uint32_t sig_seq;

static void on_signal(int sig) {
    __atomic_fetch_or(&sig_pending, (uint64_t)1 << sig, __ATOMIC_SEQ_CST);
    __atomic_fetch_add(&sig_seq, 1, __ATOMIC_SEQ_CST);
    syscall(SYS_futex, &sig_seq, FUTEX_WAKE_PRIVATE, INT32_MAX, NULL, NULL, 0);
}

int32_t ax_signal(int32_t sig, uint32_t action) {
    if (sig < 1 || sig > 64 || sig == SIGKILL || sig == SIGSTOP || sig == SIGSEGV || sig == SIGBUS || sig == SIGFPE)
        return AX_ERR;
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    if (action == 0) sa.sa_handler = SIG_DFL;
    else if (action == 1) sa.sa_handler = SIG_IGN;
    else if (action == 2) { sa.sa_handler = on_signal; sa.sa_flags = SA_RESTART; }
    else return AX_ERR;
    if (sigaction(sig, &sa, NULL)) return AX_ERR;
    sig_actions[sig] = (uint8_t)action;
    return 0;
}

int64_t ax_signal_take(int64_t timeout_ns) {
    for (;;) {
        uint32_t seq = __atomic_load_n(&sig_seq, __ATOMIC_SEQ_CST);
        uint64_t got = __atomic_exchange_n(&sig_pending, 0, __ATOMIC_SEQ_CST);
        if (got || timeout_ns == 0) return (int64_t)got;
        struct timespec ts = {timeout_ns / 1000000000, timeout_ns % 1000000000};
        long r = syscall(SYS_futex, &sig_seq, FUTEX_WAIT_PRIVATE, seq, timeout_ns < 0 ? NULL : &ts, NULL, 0);
        if (r < 0 && errno == ETIMEDOUT) return (int64_t)__atomic_exchange_n(&sig_pending, 0, __ATOMIC_SEQ_CST);
    }
}

// -----------------------------------------------------------------------------
// Addresses, IPv6, UDP, Unix sockets and polling (see std/internal/runtime/native.ax for
// the address records and the contract of each)
// -----------------------------------------------------------------------------

#include <poll.h>
#include <sys/un.h>

static socklen_t rec_sockaddr(const uint8_t *r, struct sockaddr_storage *ss) {
    memset(ss, 0, sizeof *ss);
    if (r[0] == 4) {
        struct sockaddr_in *a = (struct sockaddr_in *)ss;
        a->sin_family = AF_INET;
        memcpy(&a->sin_port, r + 2, 2);
        memcpy(&a->sin_addr, r + 4, 4);
        return sizeof *a;
    }
    if (r[0] == 6) {
        struct sockaddr_in6 *a = (struct sockaddr_in6 *)ss;
        a->sin6_family = AF_INET6;
        memcpy(&a->sin6_port, r + 2, 2);
        memcpy(&a->sin6_addr, r + 4, 16);
        uint32_t scope;
        memcpy(&scope, r + 20, 4);
        a->sin6_scope_id = scope;
        return sizeof *a;
    }
    return 0;
}

static int sockaddr_rec(const struct sockaddr *sa, uint8_t *r) {
    memset(r, 0, 24);
    if (sa->sa_family == AF_INET) {
        const struct sockaddr_in *a = (const struct sockaddr_in *)sa;
        r[0] = 4;
        memcpy(r + 2, &a->sin_port, 2);
        memcpy(r + 4, &a->sin_addr, 4);
        return 1;
    }
    if (sa->sa_family == AF_INET6) {
        const struct sockaddr_in6 *a = (const struct sockaddr_in6 *)sa;
        r[0] = 6;
        memcpy(r + 2, &a->sin6_port, 2);
        memcpy(r + 4, &a->sin6_addr, 16);
        uint32_t scope = a->sin6_scope_id;
        memcpy(r + 20, &scope, 4);
        return 1;
    }
    return 0;
}

int32_t ax_resolve(const uint8_t *name, size_t len, uint8_t *out, uint32_t max) {
    char host[256];
    if (copy_cstr(host, sizeof host, name, len) || !len) return AX_BAD_ADDRESS;
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, NULL, &hints, &res)) return AX_BAD_ADDRESS;
    uint32_t n = 0;
    for (struct addrinfo *a = res; a && n < max; a = a->ai_next)
        if (sockaddr_rec(a->ai_addr, out + 24 * n)) n++;
    freeaddrinfo(res);
    return n ? (int32_t)n : AX_BAD_ADDRESS;
}

int32_t ax_tcp_connect_addr(const uint8_t *r) {
    struct sockaddr_storage ss;
    socklen_t sl = rec_sockaddr(r, &ss);
    if (!sl) return AX_BAD_ADDRESS;
    int fd = socket(ss.ss_family, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) return AX_ERR;
    int c;
    do c = connect(fd, (struct sockaddr *)&ss, sl);
    while (c < 0 && errno == EINTR);
    if (c == 0) return fd;
    int e = errno;
    close(fd);
    return e == ECONNREFUSED ? AX_REFUSED : AX_ERR;
}

int32_t ax_tcp_listen_addr(const uint8_t *r) {
    struct sockaddr_storage ss;
    socklen_t sl = rec_sockaddr(r, &ss);
    if (!sl) return AX_BAD_ADDRESS;
    int fd = socket(ss.ss_family, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) return AX_ERR;
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
    if (bind(fd, (struct sockaddr *)&ss, sl) == 0 && listen(fd, 128) == 0) return fd;
    int e = errno;
    close(fd);
    return e == EADDRINUSE ? AX_ADDR_IN_USE : AX_ERR;
}

int32_t ax_sock_addr(int32_t h, uint32_t peer, uint8_t *out) {
    struct sockaddr_storage ss;
    socklen_t sl = sizeof ss;
    if ((peer ? getpeername : getsockname)(h, (struct sockaddr *)&ss, &sl)) return AX_ERR;
    return sockaddr_rec((struct sockaddr *)&ss, out) ? 0 : AX_ERR;
}

int32_t ax_udp_open(const uint8_t *r) {
    struct sockaddr_storage ss;
    socklen_t sl = rec_sockaddr(r, &ss);
    if (!sl) return AX_BAD_ADDRESS;
    int fd = socket(ss.ss_family, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (fd < 0) return AX_ERR;
    if (bind(fd, (struct sockaddr *)&ss, sl) == 0) return fd;
    int e = errno;
    close(fd);
    return e == EADDRINUSE ? AX_ADDR_IN_USE : AX_ERR;
}

intptr_t ax_udp_send(int32_t h, const uint8_t *p, size_t n, const uint8_t *r) {
    struct sockaddr_storage ss;
    socklen_t sl = rec_sockaddr(r, &ss);
    if (!sl) return AX_BAD_ADDRESS;
    ssize_t k;
    do k = sendto(h, p, n, MSG_NOSIGNAL, (struct sockaddr *)&ss, sl);
    while (k < 0 && errno == EINTR);
    if (k < 0) return errno == EAGAIN || errno == EWOULDBLOCK ? AX_TIMED_OUT : AX_ERR;
    return (intptr_t)k;
}

intptr_t ax_udp_recv(int32_t h, uint8_t *p, size_t n, uint8_t *from) {
    struct sockaddr_storage ss;
    ssize_t k;
    do {
        socklen_t sl = sizeof ss;
        k = recvfrom(h, p, n, 0, (struct sockaddr *)&ss, &sl);
    } while (k < 0 && errno == EINTR);
    if (k < 0) return errno == EAGAIN || errno == EWOULDBLOCK ? AX_TIMED_OUT : AX_ERR;
    sockaddr_rec((struct sockaddr *)&ss, from);
    return (intptr_t)k;
}

static int unix_addr(const uint8_t *path, size_t len, struct sockaddr_un *a) {
    if (!len || len >= sizeof a->sun_path || memchr(path, 0, len)) return 0;
    memset(a, 0, sizeof *a);
    a->sun_family = AF_UNIX;
    memcpy(a->sun_path, path, len);
    return 1;
}

int32_t ax_unix_connect(const uint8_t *path, size_t len) {
    struct sockaddr_un a;
    if (!unix_addr(path, len, &a)) return AX_BAD_ADDRESS;
    int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) return AX_ERR;
    if (connect(fd, (struct sockaddr *)&a, sizeof a) == 0) return fd;
    int e = errno;
    close(fd);
    return e == ECONNREFUSED || e == ENOENT ? AX_REFUSED : AX_ERR;
}

int32_t ax_unix_listen(const uint8_t *path, size_t len) {
    struct sockaddr_un a;
    if (!unix_addr(path, len, &a)) return AX_BAD_ADDRESS;
    int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) return AX_ERR;
    if (bind(fd, (struct sockaddr *)&a, sizeof a) == 0 && listen(fd, 128) == 0) return fd;
    int e = errno;
    close(fd);
    return e == EADDRINUSE ? AX_ADDR_IN_USE : AX_ERR;
}

int32_t ax_poll(uint8_t *entries, uint32_t n, int32_t timeout_ms) {
    int r;
    do r = poll((struct pollfd *)entries, n, timeout_ms);
    while (r < 0 && errno == EINTR);
    return r < 0 ? AX_ERR : r;
}

// -----------------------------------------------------------------------------
// More of the file system (see std/internal/runtime/native.ax for the contract of each)
// -----------------------------------------------------------------------------

#include <poll.h>
#include <sys/file.h>
#include <sys/inotify.h>

int32_t ax_chmod(const uint8_t *path, size_t len, uint32_t mode) {
    char b[4096];
    if (copy_cstr(b, sizeof b, path, len)) return AX_ERR;
    return chmod(b, mode & 07777) ? fs_err() : 0;
}

int32_t ax_symlink(const uint8_t *target, size_t tlen, const uint8_t *link, size_t llen) {
    char a[4096], b[4096];
    if (copy_cstr(a, sizeof a, target, tlen) || copy_cstr(b, sizeof b, link, llen)) return AX_ERR;
    return symlink(a, b) ? fs_err() : 0;
}

int32_t ax_link(const uint8_t *from, size_t flen, const uint8_t *to, size_t tlen) {
    char a[4096], b[4096];
    if (copy_cstr(a, sizeof a, from, flen) || copy_cstr(b, sizeof b, to, tlen)) return AX_ERR;
    return link(a, b) ? fs_err() : 0;
}

intptr_t ax_readlink(const uint8_t *path, size_t len, uint8_t *buf, size_t cap) {
    char p[4096], tmp[4096];
    if (copy_cstr(p, sizeof p, path, len)) return AX_ERR;
    ssize_t n = readlink(p, tmp, sizeof tmp);
    if (n < 0) return fs_err();
    if ((size_t)n <= cap) memcpy(buf, tmp, (size_t)n);
    return (intptr_t)n;
}

static struct timespec split_ns(int64_t ns) {
    int64_t s = ns / 1000000000, n = ns % 1000000000;
    if (n < 0) { n += 1000000000; s--; }
    return (struct timespec){s, n};
}

int32_t ax_utimes(const uint8_t *path, size_t len, int64_t atime_ns, int64_t mtime_ns) {
    char p[4096];
    if (copy_cstr(p, sizeof p, path, len)) return AX_ERR;
    struct timespec ts[2] = {split_ns(atime_ns), split_ns(mtime_ns)};
    return utimensat(AT_FDCWD, p, ts, 0) ? fs_err() : 0;
}

int32_t ax_ftruncate(int32_t fd, int64_t size) { return ftruncate(fd, size) ? fs_err() : 0; }

int32_t ax_fsync(int32_t fd) { return fsync(fd) ? AX_ERR : 0; }

int32_t ax_flock(int32_t fd, uint32_t op) {
    int r;
    do r = flock(fd, (int)op);
    while (r < 0 && errno == EINTR);
    if (r < 0) return errno == EWOULDBLOCK ? -11 : AX_ERR;
    return 0;
}

uintptr_t ax_mmap(int32_t fd, int64_t off, size_t len, uint32_t writable) {
    void *p = mmap(NULL, len, writable ? PROT_READ | PROT_WRITE : PROT_READ, MAP_SHARED, fd, off);
    return p == MAP_FAILED ? 0 : (uintptr_t)p;
}

int32_t ax_munmap(uintptr_t addr, size_t len) { return munmap((void *)addr, len) ? AX_ERR : 0; }

int32_t ax_msync(uintptr_t addr, size_t len) { return msync((void *)addr, len, MS_SYNC) ? AX_ERR : 0; }

int32_t ax_watch_open(void) {
    int fd = inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
    return fd < 0 ? AX_ERR : fd;
}

int32_t ax_watch_add(int32_t h, const uint8_t *path, size_t len) {
    char p[4096];
    if (copy_cstr(p, sizeof p, path, len)) return AX_ERR;
    int wd = inotify_add_watch(h, p, IN_MODIFY | IN_ATTRIB | IN_MOVED_FROM | IN_MOVED_TO | IN_CREATE | IN_DELETE |
                                         IN_DELETE_SELF | IN_MOVE_SELF);
    return wd < 0 ? fs_err() : wd;
}

int32_t ax_watch_remove(int32_t h, int32_t wd) { return inotify_rm_watch(h, wd) ? AX_ERR : 0; }

intptr_t ax_watch_read(int32_t h, uint8_t *buf, size_t cap, int32_t timeout_ms) {
    for (;;) {
        ssize_t r = read(h, buf, cap);
        if (r >= 0) return (intptr_t)r;
        if (errno == EINTR) continue;
        if (errno != EAGAIN) return AX_ERR;
        struct pollfd pfd = {h, POLLIN, 0};
        int p = poll(&pfd, 1, timeout_ms);
        if (p == 0) return 0;
        if (p < 0 && errno != EINTR) return AX_ERR;
    }
}

void ax_watch_close(int32_t h) { close(h); }

// -----------------------------------------------------------------------------
// Terminals
// -----------------------------------------------------------------------------

#include <sys/ioctl.h>
#include <termios.h>

int32_t ax_isatty(int32_t fd) { return isatty(fd) ? 1 : 0; }

int32_t ax_term_size(int32_t fd, uint32_t *out) {
    struct winsize ws;
    if (ioctl(fd, TIOCGWINSZ, &ws)) return AX_ERR;
    out[0] = ws.ws_col;
    out[1] = ws.ws_row;
    return 0;
}

static struct termios saved_termios;
static int saved_fd = -1;

int32_t ax_term_raw(int32_t fd, uint32_t enable) {
    if (!enable) {
        if (saved_fd != fd) return AX_ERR;
        saved_fd = -1;
        return tcsetattr(fd, TCSADRAIN, &saved_termios) ? AX_ERR : 0;
    }
    struct termios t;
    if (tcgetattr(fd, &t)) return AX_ERR;
    if (saved_fd < 0) { saved_termios = t; saved_fd = fd; }
    cfmakeraw(&t);
    return tcsetattr(fd, TCSADRAIN, &t) ? AX_ERR : 0;
}

static int resolve(const uint8_t *name, size_t len, uint32_t port, int passive, struct addrinfo **res) {
    char host[256], serv[16];
    if (copy_cstr(host, sizeof host, name, len)) return AX_BAD_ADDRESS;
    snprintf(serv, sizeof serv, "%u", port);
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (passive) hints.ai_flags = AI_PASSIVE;
    return getaddrinfo(len ? host : NULL, serv, &hints, res) == 0 ? 0 : AX_BAD_ADDRESS;
}

int32_t ax_tcp_connect(const uint8_t *name, size_t len, uint32_t port) {
    struct addrinfo *res;
    int rc = resolve(name, len, port, 0, &res);
    if (rc) return rc;
    int result = AX_ERR;
    for (struct addrinfo *a = res; a; a = a->ai_next) {
        int fd = socket(a->ai_family, a->ai_socktype | SOCK_CLOEXEC, a->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, a->ai_addr, a->ai_addrlen) == 0) {
            result = fd;
            break;
        }
        result = errno == ECONNREFUSED ? AX_REFUSED : AX_ERR;
        close(fd);
    }
    freeaddrinfo(res);
    return result;
}

int32_t ax_tcp_listen(const uint8_t *name, size_t len, uint32_t port) {
    struct addrinfo *res;
    int rc = resolve(name, len, port, 1, &res);
    if (rc) return rc;
    int result = AX_ERR;
    for (struct addrinfo *a = res; a; a = a->ai_next) {
        int fd = socket(a->ai_family, a->ai_socktype | SOCK_CLOEXEC, a->ai_protocol);
        if (fd < 0) continue;
        int one = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
        if (bind(fd, a->ai_addr, a->ai_addrlen) == 0 && listen(fd, 128) == 0) {
            result = fd;
            break;
        }
        result = errno == EADDRINUSE ? AX_ADDR_IN_USE : AX_ERR;
        close(fd);
    }
    freeaddrinfo(res);
    return result;
}

int32_t ax_tcp_accept(int32_t listener) {
    int fd;
    do fd = accept4(listener, NULL, NULL, SOCK_CLOEXEC);
    while (fd < 0 && errno == EINTR);
    return fd < 0 ? AX_ERR : fd;
}

int32_t ax_tcp_local_port(int32_t h) {
    struct sockaddr_storage ss;
    socklen_t sl = sizeof ss;
    if (getsockname(h, (struct sockaddr *)&ss, &sl) != 0) return AX_ERR;
    if (ss.ss_family == AF_INET) return ntohs(((struct sockaddr_in *)&ss)->sin_port);
    if (ss.ss_family == AF_INET6) return ntohs(((struct sockaddr_in6 *)&ss)->sin6_port);
    return AX_ERR;
}

intptr_t ax_sock_read(int32_t h, uint8_t *p, size_t n) {
    ssize_t r;
    do r = recv(h, p, n, 0);
    while (r < 0 && errno == EINTR);
    if (r < 0) return errno == EAGAIN || errno == EWOULDBLOCK ? AX_TIMED_OUT : AX_ERR;
    return (intptr_t)r;
}

intptr_t ax_sock_write(int32_t h, const uint8_t *p, size_t n) {
    ssize_t r;
    do r = send(h, p, n, MSG_NOSIGNAL);
    while (r < 0 && errno == EINTR);
    if (r < 0) return errno == EAGAIN || errno == EWOULDBLOCK ? AX_TIMED_OUT : AX_ERR;
    return (intptr_t)r;
}

// Reads and writes on h fail with AX_TIMED_OUT after ms milliseconds (0: never).
int32_t ax_sock_timeout(int32_t h, uint32_t ms) {
    struct timeval tv = {(time_t)(ms / 1000), (suseconds_t)(ms % 1000) * 1000};
    if (setsockopt(h, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv) != 0) return AX_ERR;
    if (setsockopt(h, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv) != 0) return AX_ERR;
    return 0;
}

// Shutting the socket down first wakes a thread blocked in accept on it.
void ax_sock_close(int32_t h) {
    shutdown(h, SHUT_RDWR);
    close(h);
}

// ---------------------------------------------------------------------------
// Threads (std/thread) and wait/notify (@atomic_wait / @atomic_notify)
// ---------------------------------------------------------------------------
typedef void (*ax_entry)(void *);

struct thread_start {
    ax_entry entry;
    void *arg;
    uint32_t *exited;
};

void ax_install_crash_stack(void);

int32_t ax_notify32(uint32_t *p, int32_t count);

static void *install_alt_stack(void);

static void *thread_main(void *p) {
    struct thread_start st = *(struct thread_start *)p;
    free(p);
    void *alt = install_alt_stack();
    st.entry(st.arg);
    __atomic_store_n(st.exited, 1, __ATOMIC_SEQ_CST);
    ax_notify32(st.exited, 1);
    /* the signal stack goes with the thread */
    stack_t off = {.ss_flags = SS_DISABLE};
    sigaltstack(&off, NULL);
    free(alt);
    return NULL;
}

// stack_top is ignored: native threads get their stack from the OS.
int32_t ax_thread_spawn(ax_entry entry, void *arg, size_t stack_top, uint32_t *exited) {
    (void)stack_top;
    static int32_t next_id = 1;
    struct thread_start *st = malloc(sizeof *st);
    if (!st) return AX_ERR;
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
        return AX_ERR;
    }
    return __atomic_fetch_add(&next_id, 1, __ATOMIC_SEQ_CST);
}

// Native threads need no stack from the program.
size_t ax_thread_stack_size(void) { return 0; }

// The operating system: 1 Linux, 2 macOS, 3 Windows, 0 another.
int32_t ax_os(void) {
#if defined(__linux__)
    return 1;
#elif defined(__APPLE__)
    return 2;
#elif defined(_WIN32)
    return 3;
#else
    return 0;
#endif
}

// The processor's extensions as std/os/cpu.ax's A_* bits, on AArch64 Linux
// (the kernel's hardware capabilities). x86-64 programs ask cpuid: 0.
#if defined(__aarch64__) && defined(__linux__)
#include <sys/auxv.h>
#endif
uint64_t ax_cpu_features(void) {
#if defined(__aarch64__) && defined(__linux__)
    unsigned long h = getauxval(AT_HWCAP), h2 = getauxval(AT_HWCAP2);
    // ATOMICS, LRCPC, ASIMDDP, AES, SHA2, SHA3, SHA512, SM4, CRC32, ASIMDRDM, ASIMDHP, ASIMDFHM, SVE, DIT
    static const unsigned char map[][2] = {{8, 0}, {15, 1}, {20, 2}, {3, 4}, {6, 5}, {17, 6}, {21, 7},
                                           {19, 8}, {7, 9}, {12, 10}, {10, 11}, {23, 12}, {22, 15}, {24, 17}};
    uint64_t f = 0;
    for (size_t i = 0; i < sizeof map / sizeof *map; i++)
        if ((h >> map[i][0]) & 1) f |= (uint64_t)1 << map[i][1];
    if (((h >> 3) & 1) && !((h >> 4) & 1)) f &= ~(uint64_t)0x10;  // AES without PMULL
    if (((h >> 6) & 1) && !((h >> 5) & 1)) f &= ~(uint64_t)0x20;  // SHA2 without SHA1
    // hwcap2: SVE2 1, I8MM 13, BF16 14, SME 23, CSSC 34
    static const unsigned char map2[][2] = {{1, 16}, {13, 14}, {14, 13}, {23, 18}, {34, 3}};
    for (size_t i = 0; i < sizeof map2 / sizeof *map2; i++)
        if ((h2 >> map2[i][0]) & 1) f |= (uint64_t)1 << map2[i][1];
    return f;
#else
    return 0;
#endif
}

uint32_t ax_cpu_count(void) {
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return n > 0 ? (uint32_t)n : 1;
}

void ax_sleep_ns(uint64_t ns) {
    struct timespec ts = {(time_t)(ns / 1000000000u), (long)(ns % 1000000000u)};
    while (nanosleep(&ts, &ts) != 0 && errno == EINTR) {
    }
}

// Fills p[0..n] with cryptographically secure random bytes; 0 on success.
int32_t ax_random(uint8_t *p, size_t n) {
    size_t done = 0;
    while (done < n) {
        long r = syscall(SYS_getrandom, p + done, n - done, 0);
        if (r < 0 && errno == EINTR) continue;
        if (r <= 0) return AX_ERR;
        done += (size_t)r;
    }
    return 0;
}

// Nanoseconds on clock 0 (real time since 1970) or 1 (monotonic).
int64_t ax_clock_ns(uint32_t clock) {
    struct timespec ts;
    clock_gettime(clock == 1 ? CLOCK_MONOTONIC : CLOCK_REALTIME, &ts);
    return (int64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
}

// Same results as WebAssembly memory.atomic.wait32: 0 woken, 1 not equal, 2 timed out.
int32_t ax_wait32(uint32_t *p, uint32_t expected, int64_t timeout_ns) {
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

int32_t ax_notify32(uint32_t *p, int32_t count) {
    long r = syscall(SYS_futex, p, FUTEX_WAKE_PRIVATE, count < 0 ? 0x7fffffff : count, NULL, NULL, 0);
    return r < 0 ? 0 : (int32_t)r;
}

// Linear memory (@memory_size, @memory_grow): 64 KiB pages committed from one
// region reserved at the first use. The region is as large as the system
// allows (up to 1 TiB of address space) and has no access, so reserving it
// costs no memory and counts against no limit on memory; @memory_grow makes
// its next pages read-write, which is when the kernel commits them, and it
// supplies physical pages only when they are touched.
static uintptr_t mem_base;       // 64 KiB-aligned start of the region
static size_t mem_limit;         // its size in pages
static size_t mem_pages;         // pages handed out
static uint32_t mem_state;       // 0: not reserved, 1: reserving, 2: ready, 3: failed
static pthread_mutex_t mem_lock = PTHREAD_MUTEX_INITIALIZER;  // held while pages are committed

static void mem_reserve(void) {
    uint32_t zero = 0;
    if (__atomic_compare_exchange_n(&mem_state, &zero, 1, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        for (size_t size = (size_t)1 << 40; size >= ((size_t)1 << 28); size /= 2) {
            void *p = mmap(NULL, size + 65536, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
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

intptr_t ax_memory_size(void) {
    if (__atomic_load_n(&mem_state, __ATOMIC_SEQ_CST) != 2) mem_reserve();
    return (intptr_t)(mem_base / 65536 + __atomic_load_n(&mem_pages, __ATOMIC_SEQ_CST));
}

// The previous size in pages (as ax_memory_size counts them), or -1.
intptr_t ax_memory_grow(intptr_t n) {
    if (__atomic_load_n(&mem_state, __ATOMIC_SEQ_CST) != 2) mem_reserve();
    if (mem_state != 2 || n < 0) return -1;
    pthread_mutex_lock(&mem_lock);
    size_t old = mem_pages;
    if ((size_t)n > mem_limit - old ||
        (n > 0 && mprotect((void *)(mem_base + old * 65536), (size_t)n * 65536, PROT_READ | PROT_WRITE) != 0)) {
        pthread_mutex_unlock(&mem_lock);
        return -1;
    }
    __atomic_store_n(&mem_pages, old + (size_t)n, __ATOMIC_SEQ_CST);
    pthread_mutex_unlock(&mem_lock);
    return (intptr_t)(mem_base / 65536 + old);
}

// @memory_discard: the whole 64 KiB pages in the range go back to the system
// (they stay read-write, and read as zero when next touched). Only pages of
// @memory_grow's region: anything else (the stack, globals, code, C's heap)
// is left alone.
void ax_memory_discard(uintptr_t addr, size_t len) {
    if (__atomic_load_n(&mem_state, __ATOMIC_SEQ_CST) != 2) return;
    uintptr_t lo = mem_base, hi = mem_base + __atomic_load_n(&mem_pages, __ATOMIC_SEQ_CST) * 65536;
    uintptr_t end = addr + len < addr ? UINTPTR_MAX : addr + len;
    if (addr >= hi || end <= lo) return;
    uintptr_t a = ((addr > lo ? addr : lo) + 65535) & ~(uintptr_t)65535;
    uintptr_t b = (end < hi ? end : hi) & ~(uintptr_t)65535;
    if (a < b) madvise((void *)a, b - a, MADV_DONTNEED);
}

// --- Stack traces from frame pointers --------------------------------------
// On x86-64 Linux the compiler keeps frame pointers instead of maintaining the
// shadow stack's call records, and puts an entry for each call in the section
// axiom_rets: the label just after the call (at its return address, or a
// few bytes on, past copies of the result) and the call's chain of site
// records, innermost first, null-terminated (see codegen: frame_tables_emit).
// Both are stored relative to the entry; the first walk turns them into
// addresses and sorts the table. A walk follows the frame-pointer chain and
// fills the calling thread's shadow stack (calls and depth) as the shadow
// stack would have them, so reports are formatted the same way. Programs
// built with a shadow stack (profiles, --sanitize=address) have no table,
// and their shadow stack is left alone.
struct ax_ret_entry {
    intptr_t ret, chain;
};
extern struct ax_ret_entry __start_axiom_rets[] __attribute__((weak, visibility("hidden")));
extern struct ax_ret_entry __stop_axiom_rets[] __attribute__((weak, visibility("hidden")));
__attribute__((weak)) const struct ax_shadow_stack *axiom_shadow_stack(void);

static int ax_rets_state;  // 0: as linked, 1: being prepared, 2: absolute and sorted

static void ax_rets_sift(struct ax_ret_entry *a, size_t i, size_t n) {
    for (;;) {
        size_t c = 2 * i + 1;
        if (c >= n) return;
        if (c + 1 < n && (uintptr_t)a[c + 1].ret > (uintptr_t)a[c].ret) c++;
        if ((uintptr_t)a[c].ret <= (uintptr_t)a[i].ret) return;
        struct ax_ret_entry t = a[i];
        a[i] = a[c];
        a[c] = t;
        i = c;
    }
}

static size_t ax_rets_prepare(void) {
    struct ax_ret_entry *a = __start_axiom_rets;
    size_t n = a && __stop_axiom_rets > a ? (size_t)(__stop_axiom_rets - a) : 0;
    if (!n) return 0;
    int st = 0;
    if (__atomic_compare_exchange_n(&ax_rets_state, &st, 1, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)) {
        for (size_t i = 0; i < n; i++) {
            a[i].ret += (intptr_t)&a[i].ret;
            a[i].chain += (intptr_t)&a[i].chain;
        }
        for (size_t i = n / 2; i-- > 0;) ax_rets_sift(a, i, n);  // heapsort (no allocation)
        for (size_t e = n; e-- > 1;) {
            struct ax_ret_entry t = a[0];
            a[0] = a[e];
            a[e] = t;
            ax_rets_sift(a, 0, e);
        }
        __atomic_store_n(&ax_rets_state, 2, __ATOMIC_RELEASE);
    } else {
        while (__atomic_load_n(&ax_rets_state, __ATOMIC_ACQUIRE) != 2) sched_yield();
    }
    return n;
}

// The chain of the call returning to `ra`, or NULL (not a recorded call).
static const struct ax_trap_site *const *ax_rets_find(uintptr_t ra, size_t n) {
    struct ax_ret_entry *a = __start_axiom_rets;
    size_t lo = 0, hi = n;  // the first label at or after ra
    while (lo < hi) {
        size_t m = lo + (hi - lo) / 2;
        if ((uintptr_t)a[m].ret < ra) lo = m + 1;
        else hi = m;
    }
    if (lo == n || (uintptr_t)a[lo].ret - ra > 64) return NULL;
    return (const struct ax_trap_site *const *)a[lo].chain;
}

// Fills the calling thread's shadow stack from the frames, starting at the
// frame record `fp` ({caller's frame pointer, return address}); `sp` is the
// lowest address the walk may read.
static void ax_walk_frames(uintptr_t fp, uintptr_t sp) {
    struct ax_shadow_stack *ss = axiom_shadow_stack ? (struct ax_shadow_stack *)axiom_shadow_stack() : NULL;
    size_t n = ax_rets_prepare();
    if (!ss || !n) return;
    uintptr_t top = sp + ((uintptr_t)1 << 30);
    pthread_attr_t attr;
    void *stk;
    size_t stksz;
    if (pthread_getattr_np(pthread_self(), &attr) == 0) {
        if (pthread_attr_getstack(&attr, &stk, &stksz) == 0 && (uintptr_t)stk <= sp && sp < (uintptr_t)stk + stksz)
            top = (uintptr_t)stk + stksz;
        pthread_attr_destroy(&attr);
    }
    const struct ax_trap_site *inner[AX_SS_CAP];
    uint64_t depth = 0;
    for (uintptr_t f = fp; f >= sp && f <= top - 16 && !(f & 7);) {
        const uintptr_t *rec = (const uintptr_t *)f;
        const struct ax_trap_site *const *c = ax_rets_find(rec[1], n);
        for (; c && *c; c++) {
            if (depth < AX_SS_CAP) inner[depth] = *c;
            depth++;
        }
        if (rec[0] <= f) break;
        f = rec[0];
    }
    ss->depth = depth > UINT32_MAX ? UINT32_MAX : (uint32_t)depth;
    for (uint64_t k = 0; k < depth && k < AX_SS_CAP; k++) ss->calls[(depth - 1 - k) & (AX_SS_CAP - 1)] = inner[k];
}

// From axiom.trap (compiled code), with its own frame.
void ax_stack_walk(void *fp) {
    ax_walk_frames((uintptr_t)fp, (uintptr_t)fp);
}

// From a signal handler: the interrupted code's frames.
void ax_stack_walk_signal(void *ucontext) {
#if defined(__x86_64__) && defined(__linux__)
    const ucontext_t *uc = ucontext;
    if (uc) ax_walk_frames((uintptr_t)uc->uc_mcontext.gregs[REG_RBP], (uintptr_t)uc->uc_mcontext.gregs[REG_RSP]);
#else
    (void)ucontext;
#endif
}

// Defined by std/internal/runtime/profile.c when profiling: writes the samples so far.
__attribute__((weak)) void ax_profile_flush(void);

// Default report for traps in native programs (test binaries override it).
__attribute__((weak)) void ax_on_trap(const struct ax_trap_site *site, const struct ax_trap_info *info,
                                      const struct ax_shadow_stack *ss) {
    static char buf[16384];
    char msg[1024];
    ax_format_trap(msg, sizeof msg, site, info);
    int n = snprintf(buf, sizeof buf, "axiom: %s at %s:%u:%u\n", msg, ax_site_file(site), site->line, site->col);
    if (n < 0 || (size_t)n >= sizeof buf) n = 0;
    n += (int)ax_format_stack(buf + n, sizeof buf - (size_t)n, site, ss);
    (void)!write(2, buf, (size_t)n);
    if (ax_profile_flush) ax_profile_flush();
}

// Crashes that are not Axiom traps (a stack overflow, or a fault in C code
// called through the FFI) are reported with the shadow stack, then the
// signal is re-raised.
static void ax_crash(int sig, siginfo_t *si, void *uc) {
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
    if (code == 8) n = snprintf(buf, sizeof buf, "axiom: trap T0008 (stack exhausted)\n");
    else n = snprintf(buf, sizeof buf, "axiom: crash: %s (address %p)\n", what, si->si_addr);
    if (n < 0 || (size_t)n >= sizeof buf) n = 0;
    ax_stack_walk_signal(uc);
    const struct ax_shadow_stack *ss = axiom_shadow_stack ? axiom_shadow_stack() : NULL;
    n += (int)ax_format_stack(buf + n, sizeof buf - (size_t)n, NULL, ss);
    (void)!write(2, buf, (size_t)n);
    if (ax_profile_flush) ax_profile_flush();
    signal(sig, SIG_DFL);
    raise(sig);
}

// Each thread needs its own signal stack to report its stack overflow.
static void *install_alt_stack(void) {
    stack_t st = {.ss_sp = malloc(65536), .ss_size = 65536};
    if (st.ss_sp) sigaltstack(&st, NULL);
    return st.ss_sp;
}

void ax_install_crash_stack(void) {
    (void)install_alt_stack();
}

__attribute__((constructor)) static void ax_install_crash_handler(void) {
    ax_install_crash_stack();
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_sigaction = ax_crash;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
}
