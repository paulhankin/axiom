// main.c - command-line driver.
//
//   phalange build <file.ph> [options] -o <out>
//   phalange check <file.ph> [options]
//
// Options:
//   --target=wasm32|native     (default wasm32; wasm32-freestanding accepted)
//   -O0 -O1 -O2 -O3            optimization level (never changes behavior)
//   --release                  same as --overflow=saturate
//   --overflow=trap|saturate   integer overflow mode (default trap)
//   --emit=exe|obj|llvm|asm    output kind (default: exe = .wasm module / native executable)
//   --format=human|agent-json  diagnostic format
//   --std=<dir>                standard library directory
//   --link=<file>              extra C source, object or library to link (repeatable)
//   -l<lib> -L<dir> -I<dir>    C libraries, library and include directories
//   --emit-header=<path>       write a C header for the exported functions
//   -o <path>                  output path
#include <limits.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "codegen.h"
#include "diag.h"
#include "header.h"
#include "package.h"
#include "units.h"
#include "sema.h"

const char *LLVMGetDefaultTargetTriple_wrapper(void);

extern char **environ;

static void usage(void) {
    fprintf(stderr,
            "usage: phalange <build|check|test> [<file.ph> | <package dir>] [options]\n"
            "       phalange profile [<file.ph> | <package dir>] [--rate=<hz>] [--folded=<path>]\n"
            "       phalange new <name>      create a package skeleton\n"
            "       phalange cache [path|clean]   the compiled-module cache\n"
            "  test: --filter=<text> --timeout=<ms>\n"
            "  --target=wasm32|native   -O0..-O3   --release | --overflow=trap|saturate\n"
            "  --emit=exe|obj|llvm|asm  --format=human|agent-json  --std=<dir>  -o <out>\n"
            "  --stack-traces=on|off    record call sites for crash traces and profiles (default on)\n"
            "  --units=module|program   compile modules separately (cached) or as one unit\n"
            "                           (default: module at -O0/-O1, program at -O2/-O3)  --verbose\n");
}

static bool file_exists(const char *p) { return access(p, R_OK) == 0; }
static bool is_directory(const char *p) {
    struct stat st;
    return stat(p, &st) == 0 && S_ISDIR(st.st_mode);
}

static const char *find_std(const char *argv0) {
    const char *env = getenv("PHALANGE_STD");
    if (env && *env) return env;
    char exe[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof exe - 1);
    if (n <= 0) {
        if (!realpath(argv0, exe)) return "std";
    } else {
        exe[n] = 0;
    }
    char *slash = strrchr(exe, '/');
    if (slash) *slash = 0;
    static char buf[PATH_MAX + 32];
    const char *cands[] = {"/../../std", "/../std", "/std", "/../share/phalange/std"};
    for (size_t i = 0; i < sizeof cands / sizeof *cands; i++) {
        snprintf(buf, sizeof buf, "%s%s/mem.ph", exe, cands[i]);
        if (file_exists(buf)) {
            snprintf(buf, sizeof buf, "%s%s", exe, cands[i]);
            char *r = realpath(buf, NULL);
            return r ? r : buf;
        }
    }
    return "std";
}

static int run(char *const argv[]) {
    pid_t pid;
    if (posix_spawnp(&pid, argv[0], NULL, NULL, argv, environ) != 0) return -1;
    int status;
    if (waitpid(pid, &status, 0) < 0) return -1;
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static const char *find_tool(const char **names) {
    for (int i = 0; names[i]; i++) {
        if (strchr(names[i], '/')) {
            if (access(names[i], X_OK) == 0) return names[i];
            continue;
        }
        const char *path = getenv("PATH");
        if (!path) continue;
        char buf[PATH_MAX];
        const char *p = path;
        while (*p) {
            const char *e = strchr(p, ':');
            size_t len = e ? (size_t)(e - p) : strlen(p);
            snprintf(buf, sizeof buf, "%.*s/%s", (int)len, p, names[i]);
            if (access(buf, X_OK) == 0) return names[i];
            if (!e) break;
            p = e + 1;
        }
    }
    return NULL;
}

// ---------------------------------------------------------------------------
// phalange test: run the test binary and report (spec §18).
// ---------------------------------------------------------------------------
static int run_capture(char *const argv[], CharVec *out) {
    int fds[2];
    if (pipe(fds)) return -1;
    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_adddup2(&fa, fds[1], 1);
    posix_spawn_file_actions_addclose(&fa, fds[0]);
    pid_t pid;
    int rc = posix_spawnp(&pid, argv[0], &fa, NULL, argv, environ);
    posix_spawn_file_actions_destroy(&fa);
    close(fds[1]);
    if (rc != 0) {
        close(fds[0]);
        return -1;
    }
    char buf[8192];
    ssize_t n;
    while ((n = read(fds[0], buf, sizeof buf)) > 0)
        for (ssize_t i = 0; i < n; i++) vec_push(*out, buf[i]);
    close(fds[0]);
    int status;
    waitpid(pid, &status, 0);
    vec_push(*out, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
}

static char *unescape(char *s) {
    char *w = s;
    for (char *r = s; *r; r++) {
        if (*r == '\\' && r[1]) {
            r++;
            *w++ = *r == 't' ? '\t' : *r == 'n' ? '\n' : *r == 'r' ? '\r' : *r;
        } else {
            *w++ = *r;
        }
    }
    *w = 0;
    return s;
}

static void json_str_out(const char *s) {
    putchar('"');
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '"') fputs("\\\"", stdout);
        else if (c == '\\') fputs("\\\\", stdout);
        else if (c == '\n') fputs("\\n", stdout);
        else if (c == '\r') fputs("\\r", stdout);
        else if (c == '\t') fputs("\\t", stdout);
        else if (c < 0x20) printf("\\u%04x", c);
        else putchar(c);
    }
    putchar('"');
}

static const char *source_line(Span sp) {
    SourceFile *f = g_files.data[sp.file];
    uint32_t a = sp.start, b = sp.start;
    while (a > 0 && f->text[a - 1] != '\n') a--;
    while (b < f->len && f->text[b] != '\n') b++;
    while (a < b && (f->text[a] == ' ' || f->text[a] == '\t')) a++;
    return arena_strndup(f->text + a, b - a);
}

// Site records name files by a stable key (codegen: file_key); -1 if none.
static int file_from_key(const char *text) {
    char *end;
    long long k = strtoll(text, &end, 10);
    if (end == text || k < 0 || k >= 0xFFFFFFFFll) return -1;
    for (int i = 0; i < g_files.len; i++)
        if (file_key(i) == (uint32_t)k) return i;
    return -1;
}

// Stack frames from the test runner (see runtime/test_main.c: encode_stack).
typedef struct TFrame {
    char kind;  // T trap site, C call, U innermost function (no location), E error return, R repeat, O omitted
    long count;
    Span sp;
    bool has_span;
    const char *fn;
} TFrame;

static int parse_frames(char *s, TFrame *out, int cap) {
    int n = 0;
    while (s && *s && n < cap) {
        char *end = strchr(s, '\x1e');
        if (end) *end = 0;
        char *f[5] = {0};
        int nf = 0;
        for (char *p = s; nf < 5;) {
            f[nf++] = p;
            char *t = strchr(p, '\x1f');
            if (!t) break;
            *t = 0;
            p = t + 1;
        }
        if (nf == 5) {
            TFrame fr = {.kind = f[0][0], .fn = f[4]};
            if (fr.kind == 'R' || fr.kind == 'O') {
                fr.count = strtol(f[1], NULL, 10);
            } else if (file_from_key(f[1]) >= 0) {
                fr.sp = (Span){file_from_key(f[1]), (uint32_t)strtoul(f[2], NULL, 10), (uint32_t)strtoul(f[3], NULL, 10)};
                fr.has_span = fr.kind != 'U';
            }
            out[n++] = fr;
        }
        s = end ? end + 1 : NULL;
    }
    return n;
}

static void print_frames_human(TFrame *fr, int n, bool errors) {
    if (!n) return;
    printf(errors ? "     error return trace (oldest first):\n" : "     stack trace (innermost first):\n");
    for (int i = 0; i < n; i++) {
        TFrame *f = &fr[i];
        if (f->kind == 'R') {
            printf("       ... the frame above repeats %ld more time%s\n", f->count, f->count == 1 ? "" : "s");
        } else if (f->kind == 'O') {
            printf("       ... %ld more frames\n", f->count);
        } else if (!f->has_span) {
            printf("       at %s\n", f->fn);
        } else {
            int sl, sc, el, ec;
            span_linecol(f->sp, &sl, &sc, &el, &ec);
            if (errors)
                printf("       %s:%d:%d in %s: %s\n", g_files.data[f->sp.file]->display, sl, sc, f->fn, source_line(f->sp));
            else
                printf("       at %s (%s:%d:%d)\n", f->fn, g_files.data[f->sp.file]->display, sl, sc);
        }
    }
}

static void print_frames_json(TFrame *fr, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        TFrame *f = &fr[i];
        if (i) printf(",");
        if (f->kind == 'R') {
            printf("{\"repeat\":%ld}", f->count);
            continue;
        }
        if (f->kind == 'O') {
            printf("{\"omitted\":%ld}", f->count);
            continue;
        }
        printf("{\"function\":");
        json_str_out(f->fn);
        if (f->has_span) {
            int sl, sc, el, ec;
            span_linecol(f->sp, &sl, &sc, &el, &ec);
            printf(",\"file\":");
            json_str_out(g_files.data[f->sp.file]->display);
            printf(",\"line\":%d,\"col\":%d,\"context\":", sl, sc);
            json_str_out(source_line(f->sp));
        }
        printf("}");
    }
    printf("]");
}

static int run_tests(const char *bin, const char *std_dir, const char *filter, const char *timeout, bool json) {
    Vec(char *) av = {0};
    if (g_target.wasm) {
        const char *names[] = {"node", NULL};
        const char *node = find_tool(names);
        if (!node) {
            diag_emit_fatal(json, "E9003", "phalange test needs node to run WebAssembly tests (or use --target=native)");
            return 2;
        }
        vec_push(av, (char *)node);
        vec_push(av, arena_printf("%s/../runtime/test_runner.mjs", std_dir));
    }
    vec_push(av, (char *)bin);
    if (filter) vec_push(av, (char *)filter);
    if (timeout) vec_push(av, (char *)timeout);
    vec_push(av, NULL);
    CharVec out = {0};
    int rc = run_capture(av.data, &out);
    if (rc != 0 || !out.data) {
        if (out.data) fputs(out.data, stderr);
        diag_emit_fatal(json, "E9006", "the test runner failed");
        return 2;
    }
    int passed = 0, failed = 0;
    if (json) printf("{\"schema\":\"phalange-tests/1\",\"tests\":[");
    int nres = 0;
    char *line = out.data;
    while (line && *line) {
        char *nl = strchr(line, '\n');
        if (nl) *nl = 0;
        if (strncmp(line, "PHT\t", 4) != 0) {  // stray output
            fprintf(stderr, "%s\n", line);
            line = nl ? nl + 1 : NULL;
            continue;
        }
        char *f[12] = {0};
        int nf = 0;
        for (char *p = line; nf < 12;) {
            f[nf++] = p;
            char *t = strchr(p, '\t');
            if (!t) break;
            *t = 0;
            p = t + 1;
        }
        line = nl ? nl + 1 : NULL;
        if (nf < 10) continue;
        int idx = atoi(f[1]);
        if (idx < 0 || idx >= g_prog.tests.len) continue;
        Decl *d = g_prog.tests.data[idx];
        bool pass = !strcmp(f[2], "pass");
        const char *code = f[3];
        int file_id = file_from_key(f[4]);
        Span sp = d->name_span;
        if (file_id >= 0 && file_id < g_files.len)
            sp = (Span){file_id, (uint32_t)strtoul(f[5], NULL, 10), (uint32_t)strtoul(f[6], NULL, 10)};
        const char *ms = f[7];
        char *msg = unescape(f[8]);
        char *output = unescape(f[9] ? f[9] : (char *)"");
        TFrame frames[128];
        int nframes = parse_frames(f[10] ? unescape(f[10]) : NULL, frames, 128);
        bool etrace = nframes && frames[0].kind == 'E';
        const char *tfile = g_files.data[d->name_span.file]->display;
        int sl, sc, el, ec;
        span_linecol(sp, &sl, &sc, &el, &ec);
        pass ? passed++ : failed++;
        if (json) {
            printf("%s\n  {\"name\":", nres++ ? "," : "");
            json_str_out(d->test_name);
            printf(",\"file\":");
            json_str_out(tfile);
            printf(",\"status\":\"%s\",\"duration_ms\":%s", pass ? "pass" : "fail", ms);
            if (!pass) {
                printf(",\"failure\":{\"code\":");
                json_str_out(code);
                printf(",\"message\":");
                json_str_out(msg);
                printf(",\"file\":");
                json_str_out(g_files.data[sp.file]->display);
                printf(",\"span\":{\"start_line\":%d,\"start_col\":%d,\"end_line\":%d,\"end_col\":%d,"
                       "\"start_byte\":%u,\"end_byte\":%u},\"context\":",
                       sl, sc, el, ec, sp.start, sp.end);
                json_str_out(source_line(sp));
                if (nframes) {
                    printf(etrace ? ",\"error_trace\":" : ",\"stack\":");
                    print_frames_json(frames, nframes);
                }
                printf("}");
            }
            printf(",\"output\":");
            json_str_out(output);
            printf("}");
        } else {
            printf("%s %s (%s ms)\n", pass ? "ok  " : "FAIL", d->test_name, ms);
            if (!pass) {
                printf("     %s:%d:%d: %s %s\n", g_files.data[sp.file]->display, sl, sc, code, msg);
                printf("         %s\n", source_line(sp));
                print_frames_human(frames, nframes, etrace);
                if (*output) {
                    printf("     output:\n");
                    for (char *o = output; *o;) {
                        char *e = strchr(o, '\n');
                        printf("       | %.*s\n", e ? (int)(e - o) : (int)strlen(o), o);
                        if (!e) break;
                        o = e + 1;
                    }
                }
            }
        }
    }
    if (json)
        printf("%s],\"passed\":%d,\"failed\":%d,\"status\":\"%s\"}\n", nres ? "\n" : "", passed, failed,
               failed ? "fail" : "pass");
    else
        printf("\n%d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}

// ---------------------------------------------------------------------------
// phalange profile: run a program under the sampling profiler (spec §17.6)
// and report where its time goes.
// ---------------------------------------------------------------------------
typedef struct ProfFn {
    Sym name;
    long self, total, stamp;
} ProfFn;

typedef struct ProfSite {
    Span sp;
    Sym caller, callee;
    long total, stamp;
} ProfSite;

typedef struct ProfStack {
    Sym key;
    long count;
} ProfStack;

static int cmp_fn(const void *a, const void *b) {
    const ProfFn *x = *(ProfFn *const *)a, *y = *(ProfFn *const *)b;
    if (x->self != y->self) return x->self < y->self ? 1 : -1;
    if (x->total != y->total) return x->total < y->total ? 1 : -1;
    return strcmp(x->name, y->name);
}
static int cmp_site(const void *a, const void *b) {
    const ProfSite *x = *(ProfSite *const *)a, *y = *(ProfSite *const *)b;
    if (x->total != y->total) return x->total < y->total ? 1 : -1;
    if (x->sp.file != y->sp.file) return x->sp.file - y->sp.file;
    return (int)x->sp.start - (int)y->sp.start;
}
static int cmp_stack(const void *a, const void *b) { return strcmp((*(ProfStack *const *)a)->key, (*(ProfStack *const *)b)->key); }

static int spawn_program(char *const argv[], char *const envp[], bool out_to_err) {
    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    if (out_to_err) posix_spawn_file_actions_adddup2(&fa, 2, 1);
    pid_t pid;
    int rc = posix_spawnp(&pid, argv[0], &fa, NULL, argv, envp);
    posix_spawn_file_actions_destroy(&fa);
    if (rc != 0) return -1;
    int status;
    if (waitpid(pid, &status, 0) < 0) return -1;
    return WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
}

static int run_profile(const char *bin, const char *std_dir, const char *folded_path, int hz, bool json) {
    const char *tmpdir = getenv("TMPDIR");
    const char *raw = arena_printf("%s/phalange-profile-%d.raw", tmpdir && *tmpdir ? tmpdir : "/tmp", (int)getpid());
    unlink(raw);
    Vec(char *) av = {0};
    Vec(char *) env = {0};
    for (char **e = environ; *e; e++) vec_push(env, *e);
    if (g_target.wasm) {
        const char *names[] = {"node", NULL};
        const char *node = find_tool(names);
        if (!node) {
            diag_emit_fatal(json, "E9003", "phalange profile needs node to run WebAssembly programs (or use --target=native)");
            return 2;
        }
        vec_push(av, (char *)node);
        vec_push(av, arena_printf("%s/../runtime/host.mjs", std_dir));
        vec_push(av, arena_printf("--profile=%s", raw));
        vec_push(av, arena_printf("--profile-hz=%d", hz));
    } else {
        vec_push(env, arena_printf("PHALANGE_PROFILE=%s", raw));
        vec_push(env, arena_printf("PHALANGE_PROFILE_HZ=%d", hz));
    }
    vec_push(av, (char *)bin);
    vec_push(av, NULL);
    vec_push(env, NULL);
    int status = spawn_program(av.data, env.data, json);
    size_t len;
    char *text = read_entire_file(raw, &len);
    unlink(raw);
    if (!text) {
        diag_emit_fatal(json, "E9006",
                        arena_printf("no profile was recorded (the program exited with status %d)", status));
        return 2;
    }

    // Aggregate.
    SymMap fns = {0}, sites = {0}, stacks = {0};
    Vec(ProfFn *) fnv = {0};
    Vec(ProfSite *) sitev = {0};
    Vec(ProfStack *) stackv = {0};
    long nsamples = 0, dropped = 0, stamp = 0, truncated = 0;
    const char *clock = "cpu";
    for (char *line = text; line && *line;) {
        char *nl = strchr(line, '\n');
        if (nl) *nl = 0;
        if (!strncmp(line, "hz ", 3)) hz = atoi(line + 3);
        else if (!strncmp(line, "clock ", 6)) clock = arena_strndup(line + 6, strlen(line + 6));
        else if (!strncmp(line, "dropped ", 8)) dropped = atol(line + 8);
        else if (!strncmp(line, "S\t", 2)) {
            char *f[4 + 256];
            int nf = 0;
            for (char *p = line + 2; nf < 4 + 256;) {
                f[nf++] = p;
                char *t = strchr(p, '\t');
                if (!t) break;
                *t = 0;
                p = t + 1;
            }
            if (nf >= 3) {
                long count = atol(f[0]);
                long depth = atol(f[1]);
                int ns = nf - 3;
                Sym leaf = intern_cstr(f[2]);
                // Functions root first: the callers of each site, then the leaf.
                Sym *chain = NEWN(Sym, ns + 2);
                Span *spans = NEWN(Span, ns + 1);
                int nc = 0;
                bool ok = true;
                for (int k = ns - 1; k >= 0; k--) {
                    char *g[4];
                    int ng = 0;
                    for (char *p = f[3 + k]; ng < 4;) {
                        g[ng++] = p;
                        char *t = strchr(p, '\x1f');
                        if (!t) break;
                        *t = 0;
                        p = t + 1;
                    }
                    if (ng < 4) {
                        ok = false;
                        break;
                    }
                    int fid = file_from_key(g[0]);
                    spans[nc] = (Span){fid >= 0 && fid < g_files.len ? fid : -1, (uint32_t)strtoul(g[1], NULL, 10),
                                       (uint32_t)strtoul(g[2], NULL, 10)};
                    chain[nc++] = intern_cstr(g[3]);
                }
                if (!ok) goto next;
                chain[nc] = leaf;
                nsamples += count;
                if (depth > ns) truncated += count;
                stamp++;
                for (int k = 0; k <= nc; k++) {
                    ProfFn *pf = symmap_get(&fns, chain[k]);
                    if (!pf) {
                        pf = NEW(ProfFn);
                        pf->name = chain[k];
                        symmap_put(&fns, chain[k], pf);
                        vec_push(fnv, pf);
                    }
                    if (pf->stamp != stamp) {  // once per sample, even under recursion
                        pf->stamp = stamp;
                        pf->total += count;
                    }
                    if (k == nc) pf->self += count;
                }
                for (int k = 0; k < nc; k++) {
                    if (spans[k].file < 0) continue;
                    Sym key = intern_cstr(arena_printf("%d:%u:%u:%s", spans[k].file, spans[k].start, spans[k].end, chain[k]));
                    ProfSite *ps = symmap_get(&sites, key);
                    if (!ps) {
                        ps = NEW(ProfSite);
                        ps->sp = spans[k];
                        ps->caller = chain[k];
                        ps->callee = chain[k + 1];
                        symmap_put(&sites, key, ps);
                        vec_push(sitev, ps);
                    }
                    if (ps->stamp != stamp) {
                        ps->stamp = stamp;
                        ps->total += count;
                    }
                }
                CharVec fk = {0};
                if (depth > ns)
                    for (const char *q = "[truncated];"; *q; q++) vec_push(fk, *q);
                for (int k = 0; k <= nc; k++) {
                    for (const char *q = chain[k]; *q; q++) vec_push(fk, *q == ';' ? ':' : *q);
                    if (k < nc) vec_push(fk, ';');
                }
                Sym skey = intern(fk.data, (size_t)fk.len);
                free(fk.data);
                ProfStack *st = symmap_get(&stacks, skey);
                if (!st) {
                    st = NEW(ProfStack);
                    st->key = skey;
                    symmap_put(&stacks, skey, st);
                    vec_push(stackv, st);
                }
                st->count += count;
            }
        }
    next:
        line = nl ? nl + 1 : NULL;
    }
    qsort(fnv.data, (size_t)fnv.len, sizeof *fnv.data, cmp_fn);
    qsort(sitev.data, (size_t)sitev.len, sizeof *sitev.data, cmp_site);
    qsort(stackv.data, (size_t)stackv.len, sizeof *stackv.data, cmp_stack);
    FILE *ff = fopen(folded_path, "w");
    if (ff) {
        for (int i = 0; i < stackv.len; i++) fprintf(ff, "%s %ld\n", stackv.data[i]->key, stackv.data[i]->count);
        fclose(ff);
    }
    double n = nsamples ? (double)nsamples : 1.0;
    int nfn = fnv.len < 25 ? fnv.len : 25, nsite = sitev.len < 15 ? sitev.len : 15;
    if (json) {
        printf("{\"schema\":\"phalange-profile/1\",\"samples\":%ld,\"hz\":%d,\"clock\":", nsamples, hz);
        json_str_out(clock);
        printf(",\"seconds\":%.3f,\"dropped\":%ld,\"truncated\":%ld,\"exit_status\":%d,\"folded\":", nsamples / (double)hz,
               dropped, truncated, status);
        json_str_out(ff ? folded_path : "");
        printf(",\"functions\":[");
        for (int i = 0; i < fnv.len; i++) {
            ProfFn *pf = fnv.data[i];
            printf("%s\n  {\"name\":", i ? "," : "");
            json_str_out(pf->name);
            printf(",\"self\":%ld,\"total\":%ld,\"self_pct\":%.1f,\"total_pct\":%.1f}", pf->self, pf->total,
                   100.0 * pf->self / n, 100.0 * pf->total / n);
        }
        printf("],\"call_sites\":[");
        for (int i = 0; i < sitev.len; i++) {
            ProfSite *ps = sitev.data[i];
            int sl, sc, el, ec;
            span_linecol(ps->sp, &sl, &sc, &el, &ec);
            printf("%s\n  {\"file\":", i ? "," : "");
            json_str_out(g_files.data[ps->sp.file]->display);
            printf(",\"line\":%d,\"col\":%d,\"caller\":", sl, sc);
            json_str_out(ps->caller);
            printf(",\"callee\":");
            json_str_out(ps->callee);
            printf(",\"total\":%ld,\"total_pct\":%.1f,\"context\":", ps->total, 100.0 * ps->total / n);
            json_str_out(source_line(ps->sp));
            printf("}");
        }
        printf("]}\n");
    } else {
        printf("profile: %ld samples at %d Hz (%s time, %.2f s)", nsamples, hz, clock, nsamples / (double)hz);
        if (status) printf("; the program exited with status %d", status);
        printf("\n\n  self%%  total%%  function\n");
        for (int i = 0; i < nfn; i++)
            printf("%7.1f %7.1f  %s\n", 100.0 * fnv.data[i]->self / n, 100.0 * fnv.data[i]->total / n, fnv.data[i]->name);
        if (fnv.len > nfn) printf("  ... %d more functions\n", fnv.len - nfn);
        if (nsite) printf("\n total%%  call site\n");
        for (int i = 0; i < nsite; i++) {
            ProfSite *ps = sitev.data[i];
            int sl, sc, el, ec;
            span_linecol(ps->sp, &sl, &sc, &el, &ec);
            printf("%7.1f  %s:%d:%d  %s -> %s\n", 100.0 * ps->total / n, g_files.data[ps->sp.file]->display, sl, sc,
                   ps->caller, ps->callee);
        }
        if (truncated) printf("\n%ld samples were deeper than 256 calls; their outer frames are not known\n", truncated);
        if (dropped) printf("%ld samples were dropped (sample buffer full)\n", dropped);
        if (ff) printf("\nfolded stacks: %s (for flamegraph.pl, inferno or speedscope)\n", folded_path);
    }
    return status == 0 ? 0 : 1;
}

int main(int argc, char **argv) {
    if (argc >= 2 && !strcmp(argv[1], "cache")) {
        // phalange cache [path|clean]: the object cache of separate compilation
        const char *dir = arena_printf("%s/obj", pkg_cache_dir());
        if (argc >= 3 && !strcmp(argv[2], "clean")) {
            char *rm[] = {"rm", "-rf", (char *)dir, NULL};
            int rc = run(rm);
            if (rc == 0) printf("removed %s\n", dir);
            return rc == 0 ? 0 : 1;
        }
        if (argc >= 3 && strcmp(argv[2], "path")) {
            usage();
            return 2;
        }
        printf("%s\n", dir);
        return 0;
    }
    if (argc >= 2 && !strcmp(argv[1], "new")) {
        bool js = argc >= 4 && !strcmp(argv[3], "--format=agent-json");
        if (argc < 3) {
            usage();
            return 2;
        }
        return pkg_new(argv[2], js) ? 0 : 1;
    }
    if (argc < 2) {
        usage();
        return 2;
    }
    const char *cmd = argv[1];
    bool testing = strcmp(cmd, "test") == 0;
    bool profiling = strcmp(cmd, "profile") == 0;
    bool build = strcmp(cmd, "build") == 0 || testing || profiling;
    g_test_mode = testing;
    if (!build && strcmp(cmd, "check") != 0) {
        usage();
        return 2;
    }
    const char *input = NULL, *out = NULL, *std_dir = NULL, *target = "wasm32", *emit = "exe";
    bool json = false;
    CgOptions co = {.stack_traces = true};
    Vec(char *) link_inputs = {0};
    Vec(char *) link_flags = {0};  // -l / -L
    Vec(char *) inc_flags = {0};   // -I, for C sources
    const char *header_path = NULL;
    const char *test_filter = NULL, *test_timeout = NULL;
    int prof_hz = 999;
    int units_mode = -1;  // -1: by optimization level
    bool verbose = false;
    const char *folded_path = NULL;
    for (int i = 2; i < argc; i++) {
        const char *a = argv[i];
        if (!strcmp(a, "-o") && i + 1 < argc) out = argv[++i];
        else if (!strncmp(a, "--target=", 9)) target = a + 9;
        else if (!strcmp(a, "-O0") || !strcmp(a, "-O1") || !strcmp(a, "-O2") || !strcmp(a, "-O3")) co.opt_level = a[2] - '0';
        else if (!strcmp(a, "--release")) co.overflow_saturate = true;
        else if (!strcmp(a, "--overflow=trap")) co.overflow_saturate = false;
        else if (!strcmp(a, "--overflow=saturate")) co.overflow_saturate = true;
        else if (!strncmp(a, "--emit=", 7)) emit = a + 7;
        else if (!strcmp(a, "--stack-traces=on")) co.stack_traces = true;
        else if (!strcmp(a, "--stack-traces=off")) co.stack_traces = false;
        else if (!strcmp(a, "--units=module")) units_mode = 1;
        else if (!strcmp(a, "--units=program")) units_mode = 0;
        else if (!strcmp(a, "--verbose")) verbose = true;
        else if (!strcmp(a, "--format=agent-json")) json = true;
        else if (!strcmp(a, "--format=human")) json = false;
        else if (!strncmp(a, "--std=", 6)) std_dir = a + 6;
        else if (!strncmp(a, "--cpu=", 6)) co.cpu = a + 6;
        else if (!strncmp(a, "--link=", 7)) vec_push(link_inputs, (char *)a + 7);
        else if (!strncmp(a, "--emit-header=", 14)) header_path = a + 14;
        else if (testing && !strncmp(a, "--filter=", 9)) test_filter = a;
        else if (testing && !strncmp(a, "--timeout=", 10)) test_timeout = a;
        else if (profiling && !strncmp(a, "--rate=", 7) && atoi(a + 7) > 0) prof_hz = atoi(a + 7);
        else if (profiling && !strncmp(a, "--folded=", 9)) folded_path = a + 9;
        else if ((a[1] == 'l' || a[1] == 'L') && a[0] == '-' && a[2]) vec_push(link_flags, (char *)a);
        else if (a[0] == '-' && a[1] == 'I' && a[2]) vec_push(inc_flags, (char *)a);
        else if (a[0] == '-') {
            diag_emit_fatal(json, "E9001", arena_printf("unknown option '%s'", a));
            return 2;
        } else if (!input) input = a;
        else {
            diag_emit_fatal(json, "E9001", arena_printf("unexpected argument '%s'", a));
            return 2;
        }
    }
    // A package directory (or none: the current directory) builds the
    // package's entry module; `test` also loads all of its sources.
    StrVec pkg_files = {0};
    if (!input || is_directory(input)) {
        const char *dir = input ? input : ".";
        char mf[PATH_MAX];
        snprintf(mf, sizeof mf, "%s/phalange.toml", dir);
        if (!file_exists(mf)) {
            diag_emit_fatal(json, "E9001", input ? arena_printf("'%s' is a directory without a phalange.toml", input)
                                                 : "no input file (and no phalange.toml in the current directory)");
            return 2;
        }
        Package *pk = pkg_load_dir(dir, NULL);
        if (diag_error_count() || !pk) {
            diag_emit(json, json ? stdout : stderr);
            return 1;
        }
        input = pk->entry;
        if (testing) pkg_source_files(pk, &pkg_files);
        if (!out && !testing && !profiling) out = arena_printf("%s%s", pk->name, !strcmp(target, "native") ? "" : ".wasm");
    }
    if (!strcmp(target, "wasm32") || !strcmp(target, "wasm32-freestanding")) {
        g_target = (Target){32, true, "wasm32-unknown-unknown"};
    } else if (!strcmp(target, "wasm64") || !strcmp(target, "wasm64-freestanding")) {
        g_target = (Target){64, true, "wasm64-unknown-unknown"};
    } else if (!strcmp(target, "native")) {
        g_target = (Target){64, false, NULL};
        g_target.triple = LLVMGetDefaultTargetTriple_wrapper();
    } else {
        diag_emit_fatal(json, "E9002", arena_printf("unknown target '%s' (expected wasm32, wasm64 or native)", target));
        return 2;
    }
    if (!file_exists(input)) {
        diag_emit_fatal(json, "E0211", arena_printf("cannot read '%s'", input));
        return 2;
    }
    types_init();
    SemaOptions so = {.std_dir = std_dir ? std_dir : find_std(argv[0]),
                      .extra_files = (const char *const *)pkg_files.data,
                      .n_extra_files = pkg_files.len};
    bool ok = sema_run(input, &so);
    if (ok && header_path) {
        const char *base = strrchr(input, '/');
        if (!emit_c_header(header_path, base ? base + 1 : input)) {
            diag_emit_fatal(json, "E9005", arena_printf("cannot write %s", header_path));
            return 2;
        }
        // C sources may include the generated header
        char dir[PATH_MAX];
        snprintf(dir, sizeof dir, "%s", header_path);
        char *sl = strrchr(dir, '/');
        vec_push(inc_flags, sl ? arena_printf("-I%.*s", (int)(sl - dir), dir) : "-I.");
    }
    if (!ok || !build) {
        diag_emit(json, json ? stdout : stderr);
        return ok ? 0 : 1;
    }

    // Decide output kind.
    bool final_link = !strcmp(emit, "exe");
    if (!strcmp(emit, "obj")) co.emit = EMIT_OBJ;
    else if (!strcmp(emit, "llvm")) co.emit = EMIT_LLVM;
    else if (!strcmp(emit, "asm")) co.emit = EMIT_ASM;
    else if (final_link) co.emit = EMIT_OBJ;
    else {
        diag_emit_fatal(json, "E9001", arena_printf("unknown --emit kind '%s'", emit));
        return 2;
    }
    char test_bin[PATH_MAX];
    if (profiling) {
        if (!final_link) {
            diag_emit_fatal(json, "E9001", "phalange profile builds and runs the program; --emit is not supported");
            return 2;
        }
        co.stack_traces = true;  // the profiler samples the shadow stack
        co.profile = true;
        if (!folded_path) {
            const char *base = strrchr(input, '/');
            base = base ? base + 1 : input;
            size_t n = strlen(base);
            if (n > 3 && !strcmp(base + n - 3, ".ph")) n -= 3;
            if (out) {  // -o names the folded output too
                folded_path = out;
            } else {
                folded_path = arena_printf("%.*s.folded", (int)n, base);
            }
        }
        const char *tmpdir = getenv("TMPDIR");
        snprintf(test_bin, sizeof test_bin, "%s/phalange-profile-%d%s", tmpdir && *tmpdir ? tmpdir : "/tmp", (int)getpid(),
                 g_target.wasm ? ".wasm" : "");
        out = test_bin;
        if (!g_target.wasm) {
            const char *pr = arena_printf("%s/../runtime/profile.c", so.std_dir);
            if (!file_exists(pr)) {
                diag_emit_fatal(json, "E9003", arena_printf("cannot find the profiler runtime (%s)", pr));
                return 2;
            }
            vec_push(link_inputs, (char *)pr);
        }
    }
    if (testing) {
        if (!final_link) {
            diag_emit_fatal(json, "E9001", "phalange test builds and runs a test binary; --emit is not supported");
            return 2;
        }
        const char *tmpdir = getenv("TMPDIR");
        snprintf(test_bin, sizeof test_bin, "%s/phalange-test-%d%s", tmpdir && *tmpdir ? tmpdir : "/tmp", (int)getpid(),
                 g_target.wasm ? ".wasm" : "");
        out = test_bin;
        if (!g_target.wasm) {
            const char *tm = arena_printf("%s/../runtime/test_main.c", so.std_dir);
            if (!file_exists(tm)) {
                diag_emit_fatal(json, "E9003", arena_printf("cannot find the test runtime (%s)", tm));
                return 2;
            }
            vec_push(link_inputs, (char *)tm);
        }
    }
    if (!out) {
        const char *base = strrchr(input, '/');
        base = base ? base + 1 : input;
        size_t n = strlen(base);
        if (n > 3 && !strcmp(base + n - 3, ".ph")) n -= 3;
        const char *ext = co.emit == EMIT_LLVM ? ".ll" : co.emit == EMIT_ASM ? ".s" : !final_link ? ".o"
                          : g_target.wasm ? ".wasm" : "";
        out = arena_printf("%.*s%s", (int)n, base, ext);
    }
    for (int i = 0; i < g_prog.host_fns.len; i++)
        if (!strcmp(g_prog.host_fns.data[i]->name, "ph_thread_spawn")) g_threads = true;
    // The wasm profiler reads the shadow stack from another thread, so the
    // program runs on a shared memory.
    if (profiling && g_target.wasm) g_threads = true;
    char objpath[PATH_MAX + 64];
    if (final_link) {
        snprintf(objpath, sizeof objpath, "%s.tmp%d.o", out, (int)getpid());
        co.out_path = objpath;
    } else {
        co.out_path = out;
    }
    collect_link_names();
    // Separate compilation (spec §17.7) applies to executables; --emit of a
    // single object/IR/assembly file is always one unit.
    bool separate = final_link && (units_mode >= 0 ? units_mode == 1 : co.opt_level <= 1);
    if (separate) {
        UnitObjs objs = {0};
        UnitStats us = {0};
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        if (!build_units(&co, objpath, &objs, &us)) {
            diag_emit(json, json ? stdout : stderr);
            if (!diag_error_count()) diag_emit_fatal(json, "E9007", "code generation failed");
            unlink(objpath);
            return 1;
        }
        clock_gettime(CLOCK_MONOTONIC, &t1);
        if (verbose)
            fprintf(stderr, "phalange: %d modules, %d compiled, %d cached (%.0f ms)\n", us.total, us.compiled,
                    us.total - us.compiled, (t1.tv_sec - t0.tv_sec) * 1e3 + (t1.tv_nsec - t0.tv_nsec) / 1e6);
        for (int i = 0; i < objs.len; i++) vec_push(link_inputs, objs.data[i]);
    } else if (!codegen_program(&co)) {
        diag_emit(json, json ? stdout : stderr);
        if (final_link) unlink(objpath);
        return 1;
    }
    if (final_link) {
        int rc;
        if (g_target.wasm) {
            const char *names[] = {"wasm-ld", "wasm-ld-18", "/usr/lib/llvm-18/bin/wasm-ld", "rust-lld", NULL};
            const char *ld = find_tool(names);
            if (!ld) {
                unlink(objpath);
                diag_emit_fatal(json, "E9003", "cannot find wasm-ld (install lld)");
                return 2;
            }
            Vec(char *) av = {0};
            vec_push(av, (char *)ld);
            if (!strcmp(ld, "rust-lld")) {
                vec_push(av, "-flavor");
                vec_push(av, "wasm");
            }
            vec_push(av, "--no-entry");
            vec_push(av, "--stack-first");
            vec_push(av, "-z");
            vec_push(av, "stack-size=1048576");
            vec_push(av, "--export-memory");
            if (g_threads) {
                // Threads are instances sharing one memory, which the host creates.
                vec_push(av, "--shared-memory");
                vec_push(av, "--import-memory");
                vec_push(av, "--max-memory=1073741824");
            }
            for (int i = 0; i < g_export_names.len; i++) vec_push(av, arena_printf("--export=%s", g_export_names.data[i]));
            vec_push(av, objpath);
            // C sources are compiled for wasm and linked into the same module
            // (they share the linear memory and the stack).
            Vec(char *) c_objs = {0};
            rc = 0;
            for (int i = 0; i < link_inputs.len && rc == 0; i++) {
                const char *in = link_inputs.data[i];
                size_t n = strlen(in);
                if (n > 2 && !strcmp(in + n - 2, ".c")) {
                    const char *cc_names[] = {"clang", "clang-18", "/usr/lib/llvm-18/bin/clang", NULL};
                    const char *clang = find_tool(cc_names);
                    if (!clang) {
                        unlink(objpath);
                        diag_emit_fatal(json, "E9003", "cannot find clang to compile C sources for wasm");
                        return 2;
                    }
                    char *cobj = arena_printf("%s.c%d.%d.o", out, i, (int)getpid());
                    Vec(char *) cv = {0};
                    vec_push(cv, (char *)clang);
                    vec_push(cv, g_target.ptr_bits == 64 ? "--target=wasm64-unknown-unknown" : "--target=wasm32-unknown-unknown");
                    const char *copts[] = {"-O2", "-c", "-ffreestanding", "-mbulk-memory", "-msimd128", "-matomics",
                                           "-mnontrapping-fptoint", "-msign-ext", "-mmutable-globals"};
                    for (size_t k = 0; k < sizeof copts / sizeof *copts; k++) vec_push(cv, (char *)copts[k]);
                    for (int k = 0; k < inc_flags.len; k++) vec_push(cv, inc_flags.data[k]);
                    vec_push(cv, (char *)in);
                    vec_push(cv, "-o");
                    vec_push(cv, cobj);
                    vec_push(cv, NULL);
                    rc = run(cv.data);
                    vec_push(c_objs, cobj);
                    vec_push(av, cobj);
                } else {
                    vec_push(av, (char *)in);
                }
            }
            for (int i = 0; i < link_flags.len; i++) vec_push(av, link_flags.data[i]);
            for (int i = 0; i < g_link_libs.len; i++) vec_push(av, arena_printf("-l%s", g_link_libs.data[i]));
            vec_push(av, "-o");
            vec_push(av, (char *)out);
            vec_push(av, NULL);
            if (rc == 0) rc = run(av.data);
            for (int i = 0; i < c_objs.len; i++) unlink(c_objs.data[i]);
        } else {
            // Every native executable gets the native runtime: the host ABI
            // (std/io, std/net, threads) and crash reporting (spec §17.4).
            {
                const char *rt = arena_printf("%s/../runtime/native_host.c", so.std_dir);
                if (!file_exists(rt)) {
                    unlink(objpath);
                    diag_emit_fatal(json, "E9003", arena_printf("cannot find the native runtime (%s)", rt));
                    return 2;
                }
                vec_push(link_inputs, (char *)rt);
            }
            const char *names[] = {"cc", "clang", "gcc", NULL};
            const char *cc = find_tool(names);
            if (!cc) {
                unlink(objpath);
                diag_emit_fatal(json, "E9003", "cannot find a C compiler to link with");
                return 2;
            }
            // The toolchain's runtime sources are compiled once and cached.
            const char *rtdir = arena_printf("%s/../runtime/", so.std_dir);
            for (int i = 0; i < link_inputs.len; i++) {
                const char *in = link_inputs.data[i];
                if (!strncmp(in, rtdir, strlen(rtdir))) {
                    const char *o = cached_runtime_object(cc, in);
                    if (o) link_inputs.data[i] = (char *)o;
                }
            }
            Vec(char *) av = {0};
            vec_push(av, (char *)cc);
            vec_push(av, objpath);
            for (int i = 0; i < inc_flags.len; i++) vec_push(av, inc_flags.data[i]);
            for (int i = 0; i < link_inputs.len; i++) vec_push(av, link_inputs.data[i]);
            for (int i = 0; i < link_flags.len; i++) vec_push(av, link_flags.data[i]);
            for (int i = 0; i < g_link_libs.len; i++) vec_push(av, arena_printf("-l%s", g_link_libs.data[i]));
            vec_push(av, "-o");
            vec_push(av, (char *)out);
            vec_push(av, "-lm");
            vec_push(av, "-pthread");
            vec_push(av, NULL);
            rc = run(av.data);
        }
        unlink(objpath);
        if (rc != 0) {
            diag_emit_fatal(json, "E9004", "linking failed");
            return 1;
        }
    }
    if (testing) {
        int rc = run_tests(out, so.std_dir, test_filter, test_timeout, json);
        unlink(out);
        return rc;
    }
    if (profiling) {
        int rc = run_profile(out, so.std_dir, folded_path, prof_hz, json);
        unlink(out);
        return rc;
    }
    diag_emit(json, json ? stdout : stderr);
    return 0;
}
