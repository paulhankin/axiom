// std/internal/runtime/profile.c - sampling profiler for native Axiom programs (spec
// §16.6). `axiom profile` links this in and runs the program with
// AXIOM_PROFILE=<raw output file> and AXIOM_PROFILE_HZ=<rate>.
//
// A SIGPROF timer on the process CPU clock interrupts whichever thread
// is running; the handler copies that thread's shadow stack (§16.4) into a
// preallocated buffer. At exit the samples are aggregated and written in the
// raw format read by `axiom profile`:
//
//   # axiom-profile-raw 1
//   hz <rate>
//   clock cpu
//   S <count> <depth> <leaf> <site>...        (tab-separated, sites innermost first)
//
// where each <site> is <file id>\x1f<start>\x1f<end>\x1f<function>.
#define _GNU_SOURCE
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "trap_format.h"

const struct ax_shadow_stack *axiom_shadow_stack(void);

#define BUF_WORDS (32u << 20)  // 256 MiB of address space, touched only as used
static uintptr_t *buf;
static size_t buf_pos;
static unsigned long dropped;
static int hz = 999;
static const char *out_path;
static timer_t timer;
static int timer_ok;

// Each sample: [depth (top bit: not the main thread), weight, frames...].
// The kernel may check CPU timers only at scheduler ticks; expirations it
// merged are reported as the overrun, so a sample stands for 1 + overrun
// periods.
static void on_prof(int sig, siginfo_t *si, void *uc) {
    (void)sig;
    (void)uc;
    const struct ax_shadow_stack *ss = axiom_shadow_stack();
    if (!ss) return;
    uint32_t depth = __atomic_load_n(&ss->depth, __ATOMIC_RELAXED);
    uint32_t n = depth < AX_SS_CAP ? depth : AX_SS_CAP;
    size_t at = __atomic_fetch_add(&buf_pos, (size_t)n + 2, __ATOMIC_RELAXED);
    if (at + n + 2 > BUF_WORDS) {
        __atomic_fetch_add(&dropped, 1, __ATOMIC_RELAXED);
        return;
    }
    // depth, with the top bit set for threads other than the main one
    buf[at] = depth | (gettid() == getpid() ? 0 : (uintptr_t)1 << (sizeof(uintptr_t) * 8 - 1));
    buf[at + 1] = 1 + (uintptr_t)(si->si_code == SI_TIMER && si->si_overrun > 0 ? si->si_overrun : 0);
    for (uint32_t i = 0; i < n; i++) buf[at + 2 + i] = (uintptr_t)ss->calls[(depth - 1 - i) & (AX_SS_CAP - 1)];
}

// Aggregation: an open-addressing table keyed by the sample's words.
typedef struct Entry {
    size_t at;  // offset of the sample in buf
    unsigned long count;
} Entry;

static uint64_t hash_sample(const uintptr_t *s, size_t n) {
    uint64_t h = 1469598103934665603ull;
    for (size_t i = 0; i < n; i++) h = (h ^ s[i]) * 1099511628211ull;
    return h;
}

#define THREAD_BIT ((uintptr_t)1 << (sizeof(uintptr_t) * 8 - 1))

static size_t sample_len(size_t at) {
    uintptr_t d = buf[at] & ~THREAD_BIT;
    return 2 + (d < AX_SS_CAP ? d : AX_SS_CAP);
}

static void put_site(FILE *f, const struct ax_trap_site *s) {
    fprintf(f, "\t%u\x1f%u\x1f%u\x1f%s", s->file_id, s->start, s->end, ax_site_fn(s));
}

static void write_profile(void) {
    static int written;
    if (written++ || !buf) return;
    if (timer_ok) timer_delete(timer);
    signal(SIGPROF, SIG_IGN);
    size_t end = buf_pos < BUF_WORDS ? buf_pos : BUF_WORDS;
    FILE *f = fopen(out_path, "w");
    if (!f) return;
    fprintf(f, "# axiom-profile-raw 1\nhz %d\nclock cpu\n", hz);
    if (dropped) fprintf(f, "dropped %lu\n", dropped);
    size_t cap = 1024;
    while (cap < end / 4) cap <<= 1;
    Entry *tab = calloc(cap, sizeof *tab);
    if (!tab) {
        fclose(f);
        return;
    }
    for (size_t at = 0; at < end;) {
        size_t n = sample_len(at);
        if (at + n > end) break;
        uintptr_t w = buf[at + 1];
        size_t h = (size_t)(hash_sample(buf + at, 1) ^ hash_sample(buf + at + 2, n - 2)) & (cap - 1);
        for (;;) {
            Entry *e = &tab[h];
            if (!e->count) {
                e->at = at;
                e->count = w;
                break;
            }
            if (sample_len(e->at) == n && buf[e->at] == buf[at] &&
                !memcmp(buf + e->at + 2, buf + at + 2, (n - 2) * sizeof *buf)) {
                e->count += w;
                break;
            }
            h = (h + 1) & (cap - 1);
        }
        at += n;
    }
    for (size_t i = 0; i < cap; i++) {
        if (!tab[i].count) continue;
        const uintptr_t *s = buf + tab[i].at;
        size_t n = sample_len(tab[i].at) - 2;
        const struct ax_trap_site *inner = n ? (const struct ax_trap_site *)s[2] : NULL;
        const char *leaf = inner ? (ax_site_callee(inner)[0] ? ax_site_callee(inner) : "<indirect call>")
                           : s[0] & THREAD_BIT ? "<thread entry>" : "main";
        fprintf(f, "S\t%lu\t%lu\t%s", tab[i].count, (unsigned long)(s[0] & ~THREAD_BIT), leaf);
        for (size_t k = 0; k < n; k++) put_site(f, (const struct ax_trap_site *)s[2 + k]);
        fputc('\n', f);
    }
    free(tab);
    fclose(f);
}

void ax_profile_flush(void) { write_profile(); }

__attribute__((constructor)) static void ax_profile_start(void) {
    out_path = getenv("AXIOM_PROFILE");
    if (!out_path || !*out_path) return;
    const char *r = getenv("AXIOM_PROFILE_HZ");
    if (r && atoi(r) > 0) hz = atoi(r);
    buf = mmap(NULL, BUF_WORDS * sizeof *buf, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (buf == MAP_FAILED) return;
    atexit(write_profile);
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_sigaction = on_prof;
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction(SIGPROF, &sa, NULL);
    // A timer on the process's CPU clock: unlike setitimer(ITIMER_PROF) it
    // is not limited to the scheduler tick.
    struct sigevent sev;
    memset(&sev, 0, sizeof sev);
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGPROF;
    if (timer_create(CLOCK_PROCESS_CPUTIME_ID, &sev, &timer) != 0) return;
    timer_ok = 1;
    long ns = 1000000000L / hz;
    struct itimerspec its = {{ns / 1000000000L, ns % 1000000000L}, {ns / 1000000000L, ns % 1000000000L}};
    timer_settime(timer, 0, &its, NULL);
}
