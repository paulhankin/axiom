// runtime/test_main.c - runs the tests of a native Phalange test binary.
// Each test runs in a forked child (isolation from traps, a timeout, and its
// own captured output). Output protocol: see runtime/test_runner.mjs.
#define _GNU_SOURCE
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "trap_format.h"

uint32_t phalange_test_count(void);
const char *phalange_test_name(uint32_t i);
int32_t phalange_test_expect(uint32_t i);
uint32_t phalange_test_run(uint32_t i);
const char *phalange_error_name(uint32_t code);

static int result_fd = -1;

// Called by the trapping test (in the child) before it dies.
void ph_on_trap(const struct ph_trap_site *site, const struct ph_trap_info *info) {
    char msg[1024], rec[1400];
    ph_format_trap(msg, sizeof msg, site, info);
    int n = snprintf(rec, sizeof rec, "TRAP\t%u\t%u\t%u\t%u\t%s", site->code, site->file_id, site->start, site->end, msg);
    if (result_fd >= 0 && n > 0) (void)!write(result_fd, rec, (size_t)n < sizeof rec ? (size_t)n : sizeof rec - 1);
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
    uint32_t n = phalange_test_count();
    for (uint32_t i = 0; i < n; i++) {
        if (*filter && !strstr(phalange_test_name(i), filter)) continue;
        int expect = phalange_test_expect(i);
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
            struct itimerval it = {{0, 0}, {timeout_ms / 1000, (timeout_ms % 1000) * 1000}};
            setitimer(ITIMER_REAL, &it, NULL);
            uint32_t code = phalange_test_run(i);
            char rec[64];
            int k = snprintf(rec, sizeof rec, "DONE\t%u", code);
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
        char msg[1500] = "";
        unsigned file_id = (unsigned)-1, start = 0, end = 0;
        if (resbuf && !strncmp(resbuf, "DONE\t", 5)) {
            unsigned ec = (unsigned)strtoul(resbuf + 5, NULL, 10);
            if (ec) {
                st = "fail";
                code = "error";
                snprintf(msg, sizeof msg, "test returned error.%s", phalange_error_name(ec));
            } else if (expect != -1) {
                st = "fail";
                code = "no_trap";
                if (expect == -2) snprintf(msg, sizeof msg, "expected a trap, but the test completed");
                else snprintf(msg, sizeof msg, "expected trap T%04d, but the test completed", expect);
            }
        } else if (resbuf && !strncmp(resbuf, "TRAP\t", 5)) {
            unsigned tc;
            char text[1400] = "";
            sscanf(resbuf + 5, "%u\t%u\t%u\t%u\t%1399[^\n]", &tc, &file_id, &start, &end, text);
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
                snprintf(msg, sizeof msg, "trap %s (%s, signal %d)", codebuf, ph_trap_names[tc], sig);
            }
        } else {
            st = "fail";
            code = "T0000";
            snprintf(msg, sizeof msg, "test process exited unexpectedly (status %d)", status);
        }
        printf("PHT\t%u\t%s\t%s\t%d\t%u\t%u\t%.0f\t", i, st, code, (int)file_id, start, end, ms);
        put_escaped(msg, strlen(msg));
        putchar('\t');
        if (outbuf) put_escaped(outbuf, outlen);
        putchar('\n');
        free(outbuf);
        free(resbuf);
    }
    return 0;
}
