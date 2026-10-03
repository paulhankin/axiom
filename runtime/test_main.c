// runtime/test_main.c - runs the tests of a native Axiom test binary.
// Each test runs in a forked child (isolation from traps, a timeout, and its
// own captured output). Output protocol: see runtime/test_runner.mjs.
#define _GNU_SOURCE
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#ifdef __linux__
#include <sys/prctl.h>
#endif
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "trap_format.h"

uint32_t axiom_test_count(void);
const char *axiom_test_name(uint32_t i);
int32_t axiom_test_expect(uint32_t i);
uint32_t axiom_test_run(uint32_t i);
const char *axiom_error_name(uint32_t code);
uint32_t axiom_test_leaks(void);

static int result_fd = -1;

const struct ax_shadow_stack *axiom_shadow_stack(void);

// Stack frames in the AXT protocol: frames separated by \x1e, fields by \x1f:
// kind (T trap site, C call, E error return, R repeat, O omitted), file id,
// start, end, function.
static size_t put_frame(char *out, size_t cap, char kind, const struct ax_trap_site *s, long count) {
    int k = s ? snprintf(out, cap, "%c\x1f%u\x1f%u\x1f%u\x1f%s\x1e", kind, s->file_id, s->start, s->end, s->fn)
              : snprintf(out, cap, "%c\x1f%ld\x1f" "0\x1f" "0\x1f\x1e", kind, count);
    return k > 0 && (size_t)k < cap ? (size_t)k : 0;
}

static size_t encode_stack(char *out, size_t cap, const struct ax_trap_site *top, const struct ax_shadow_stack *ss) {
    size_t n = 0;
    if (cap) out[0] = 0;
    if (top) n += put_frame(out + n, cap - n, 'T', top, 0);
    if (!ss) return n;
    uint32_t depth = ss->depth;
    int shown = 0;
    for (int64_t i = (int64_t)depth - 1; i >= 0 && i >= (int64_t)depth - AX_SS_CAP; i--) {
        const struct ax_trap_site *c = ss->calls[i & (AX_SS_CAP - 1)];
        if (!c) continue;
        if (!top && n == 0) n += put_frame(out + n, cap - n, 'U', c, 0);  // innermost function, location unknown
        if (shown >= 40) return n + put_frame(out + n, cap - n, 'O', NULL, (long)i + 1);
        int64_t j = i;
        while (j - 1 >= 0 && j - 1 >= (int64_t)depth - AX_SS_CAP && ss->calls[(j - 1) & (AX_SS_CAP - 1)] == c) j--;
        n += put_frame(out + n, cap - n, 'C', c, 0);
        shown++;
        if (j < i) {
            n += put_frame(out + n, cap - n, 'R', NULL, (long)(i - j));
            i = j;
        }
    }
    if (depth > AX_SS_CAP) n += put_frame(out + n, cap - n, 'O', NULL, (long)(depth - AX_SS_CAP));
    return n;
}

static size_t encode_errors(char *out, size_t cap, const struct ax_shadow_stack *ss) {
    size_t n = 0;
    if (cap) out[0] = 0;
    if (!ss) return 0;
    uint32_t first = ss->errors > AX_ET_CAP ? ss->errors - AX_ET_CAP : 0;
    for (uint32_t i = first; i < ss->errors; i++)
        if (ss->error_returns[i & (AX_ET_CAP - 1)]) n += put_frame(out + n, cap - n, 'E', ss->error_returns[i & (AX_ET_CAP - 1)], 0);
    return n;
}

// Called by the trapping test (in the child) before it dies.
void ax_on_trap(const struct ax_trap_site *site, const struct ax_trap_info *info, const struct ax_shadow_stack *ss) {
    static char rec[32768];
    char msg[1024];
    ax_format_trap(msg, sizeof msg, site, info);
    int n = snprintf(rec, sizeof rec, "TRAP\t%u\t%u\t%u\t%u\t%s\t", site->code, site->file_id, site->start, site->end, msg);
    if (n < 0 || (size_t)n >= sizeof rec) return;
    n += (int)encode_stack(rec + n, sizeof rec - (size_t)n, site, ss);
    if (result_fd >= 0) (void)!write(result_fd, rec, (size_t)n);
}

// Crashes other than traps (stack overflow, faults in C code): report the
// shadow stack, then die from the signal.
static void on_crash(int sig, siginfo_t *si, void *uc) {
    (void)uc;
    static char rec[32768];
    int code = 5;  // memory access out of bounds, unless the fault is at the stack's end
    pthread_attr_t attr;
    void *stk;
    size_t stksz;
    uintptr_t addr = (uintptr_t)si->si_addr;
    if (pthread_getattr_np(pthread_self(), &attr) == 0) {
        if (pthread_attr_getstack(&attr, &stk, &stksz) == 0 && addr + 65536 >= (uintptr_t)stk &&
            addr < (uintptr_t)stk + 65536)
            code = 8;
        pthread_attr_destroy(&attr);
    }
    int n = snprintf(rec, sizeof rec, "CRASH\t%d\t", code);
    n += (int)encode_stack(rec + n, sizeof rec - (size_t)n, NULL, axiom_shadow_stack());
    if (result_fd >= 0) (void)!write(result_fd, rec, (size_t)n);
    signal(sig, SIG_DFL);
    raise(sig);
}

static void put_escaped(const char *s, size_t n) {
    for (size_t i = 0; i < n; i++) {
        char c = s[i];
        if (c == '\\') fputs("\\\\", stdout);
        else if (c == '\t') fputs("\\t", stdout);
        else if (c == '\n') fputs("\\n", stdout);
        else if (c == '\r') fputs("\\r", stdout);
        else putchar(c);
    }
}

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e3 + ts.tv_nsec / 1e6;
}

int main(int argc, char **argv) {
    const char *filter = "";
    long timeout_ms = 10000;
    for (int i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "--filter=", 9)) filter = argv[i] + 9;
        else if (!strncmp(argv[i], "--timeout=", 10)) timeout_ms = atol(argv[i] + 10);
    }
    uint32_t n = axiom_test_count();
    for (uint32_t i = 0; i < n; i++) {
        if (*filter && !strstr(axiom_test_name(i), filter)) continue;
        int expect = axiom_test_expect(i);
        int res[2], out[2];
        if (pipe(res) || pipe(out)) return 2;
        fflush(stdout);
        double t0 = now_ms();
        pid_t pid = fork();
        if (pid == 0) {
            close(res[0]);
            close(out[0]);
            dup2(out[1], 1);
            dup2(out[1], 2);
            result_fd = res[1];
#ifdef __linux__
            // A trap or crash ends the child without a core dump: with a piped
            // core_pattern (as under WSL) RLIMIT_CORE is ignored, and a dump into
            // a helper that does not read it never ends.
            prctl(PR_SET_DUMPABLE, 0);
#endif
            struct itimerval it = {{0, 0}, {timeout_ms / 1000, (timeout_ms % 1000) * 1000}};
            setitimer(ITIMER_REAL, &it, NULL);
            static char altstack[65536];
            stack_t alt = {.ss_sp = altstack, .ss_size = sizeof altstack};
            sigaltstack(&alt, NULL);
            struct sigaction sa;
            memset(&sa, 0, sizeof sa);
            sa.sa_sigaction = on_crash;
            sa.sa_flags = SA_ONSTACK | SA_SIGINFO;
            sigaction(SIGSEGV, &sa, NULL);
            sigaction(SIGBUS, &sa, NULL);
            uint32_t code = axiom_test_run(i);
            static char rec[8192];
            int k = snprintf(rec, sizeof rec, "DONE\t%u\t", code);
            if (code) k += (int)encode_errors(rec + k, sizeof rec - (size_t)k, axiom_shadow_stack());
            // --sanitize=address: memory the test never freed (reported on stderr)
            else k += snprintf(rec + k, sizeof rec - (size_t)k, "%u", axiom_test_leaks());
            (void)!write(res[1], rec, (size_t)k);
            _exit(0);
        }
        close(res[1]);
        close(out[1]);
        // Drain output and result pipes together (either may fill up).
        char *outbuf = NULL, *resbuf = NULL;
        size_t outlen = 0, reslen = 0;
        struct pollfd pf[2] = {{out[0], POLLIN, 0}, {res[0], POLLIN, 0}};
        int open_fds = 2;
        while (open_fds > 0) {
            if (poll(pf, 2, -1) < 0 && errno != EINTR) break;
            for (int k = 0; k < 2; k++) {
                if (pf[k].fd < 0 || !(pf[k].revents & (POLLIN | POLLHUP | POLLERR))) continue;
                char buf[4096];
                ssize_t r = read(pf[k].fd, buf, sizeof buf);
                if (r <= 0) {
                    close(pf[k].fd);
                    pf[k].fd = -1;
                    open_fds--;
                    continue;
                }
                char **dst = k == 0 ? &outbuf : &resbuf;
                size_t *len = k == 0 ? &outlen : &reslen;
                *dst = realloc(*dst, *len + (size_t)r + 1);
                memcpy(*dst + *len, buf, (size_t)r);
                *len += (size_t)r;
                (*dst)[*len] = 0;
            }
        }
        int status = 0;
        waitpid(pid, &status, 0);
        double ms = now_ms() - t0;
        const char *st = "pass", *code = "";
        const char *stack = "";
        char msg[1500] = "";
        unsigned file_id = (unsigned)-1, start = 0, end = 0;
        if (resbuf && !strncmp(resbuf, "DONE\t", 5)) {
            char *endp;
            unsigned ec = (unsigned)strtoul(resbuf + 5, &endp, 10);
            if (ec) {
                st = "fail";
                code = "error";
                if (*endp == '\t') stack = endp + 1;
                snprintf(msg, sizeof msg, "test returned error.%s", axiom_error_name(ec));
            } else if (expect != -1) {
                st = "fail";
                code = "no_trap";
                if (expect == -2) snprintf(msg, sizeof msg, "expected a trap, but the test completed");
                else snprintf(msg, sizeof msg, "expected trap T%04d, but the test completed", expect);
            } else if (*endp == '\t') {
                unsigned long leaks = strtoul(endp + 1, NULL, 10);
                if (leaks) {
                    st = "fail";
                    code = "leak";
                    snprintf(msg, sizeof msg, "the test leaked %lu allocation%s (--sanitize=address)", leaks, leaks == 1 ? "" : "s");
                }
            }
        } else if (resbuf && !strncmp(resbuf, "TRAP\t", 5)) {
            unsigned tc;
            char text[1400] = "";
            sscanf(resbuf + 5, "%u\t%u\t%u\t%u\t%1399[^\t]", &tc, &file_id, &start, &end, text);
            char *tab = resbuf + 5;
            for (int f = 0; f < 5 && tab; f++) tab = strchr(tab + (f ? 1 : 0), '\t');
            if (tab) stack = tab + 1;
            if (expect == -2 || expect == (int)tc) {
                file_id = (unsigned)-1;
            } else {
                static char codebuf[8];
                snprintf(codebuf, sizeof codebuf, "T%04u", tc);
                st = "fail";
                code = codebuf;
                if (expect >= 0) snprintf(msg, sizeof msg, "expected trap T%04d, got %s", expect, text);
                else snprintf(msg, sizeof msg, "%s", text);
            }
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            int tc = sig == SIGSEGV || sig == SIGBUS ? 5 : 0;
            if (resbuf && !strncmp(resbuf, "CRASH\t", 6)) {
                tc = atoi(resbuf + 6);
                char *tab = strchr(resbuf + 6, '\t');
                if (tab) stack = tab + 1;
            }
            if (sig == SIGALRM) {
                st = "fail";
                code = "timeout";
                snprintf(msg, sizeof msg, "test did not finish within %ld ms", timeout_ms);
            } else if (expect == -2 || expect == tc) {
                /* pass */
            } else {
                static char codebuf[8];
                snprintf(codebuf, sizeof codebuf, "T%04d", tc);
                st = "fail";
                code = codebuf;
                snprintf(msg, sizeof msg, "trap %s (%s, signal %d)", codebuf, ax_trap_names[tc], sig);
            }
        } else {
            st = "fail";
            code = "T0000";
            snprintf(msg, sizeof msg, "test process exited unexpectedly (status %d)", status);
        }
        printf("AXT\t%u\t%s\t%s\t%lld\t%u\t%u\t%.0f\t", i, st, code, file_id == (unsigned)-1 ? -1ll : (long long)file_id, start, end, ms);
        put_escaped(msg, strlen(msg));
        putchar('\t');
        if (outbuf) put_escaped(outbuf, outlen);
        putchar('\t');
        if (strcmp(st, "pass")) put_escaped(stack, strlen(stack));
        putchar('\n');
        free(outbuf);
        free(resbuf);
    }
    return 0;
}
