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
#include <sys/wait.h>
#include <unistd.h>

#include "codegen.h"
#include "diag.h"
#include "header.h"
#include "sema.h"

const char *LLVMGetDefaultTargetTriple_wrapper(void);

extern char **environ;

static void usage(void) {
    fprintf(stderr,
            "usage: phalange <build|check> <file.ph> [options]\n"
            "  --target=wasm32|native   -O0..-O3   --release | --overflow=trap|saturate\n"
            "  --emit=exe|obj|llvm|asm  --format=human|agent-json  --std=<dir>  -o <out>\n");
}

static bool file_exists(const char *p) { return access(p, R_OK) == 0; }

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

int main(int argc, char **argv) {
    if (argc < 3) {
        usage();
        return 2;
    }
    const char *cmd = argv[1];
    bool build = strcmp(cmd, "build") == 0;
    if (!build && strcmp(cmd, "check") != 0) {
        usage();
        return 2;
    }
    const char *input = NULL, *out = NULL, *std_dir = NULL, *target = "wasm32", *emit = "exe";
    bool json = false;
    CgOptions co = {0};
    Vec(char *) link_inputs = {0};
    Vec(char *) link_flags = {0};  // -l / -L
    Vec(char *) inc_flags = {0};   // -I, for C sources
    const char *header_path = NULL;
    for (int i = 2; i < argc; i++) {
        const char *a = argv[i];
        if (!strcmp(a, "-o") && i + 1 < argc) out = argv[++i];
        else if (!strncmp(a, "--target=", 9)) target = a + 9;
        else if (!strcmp(a, "-O0") || !strcmp(a, "-O1") || !strcmp(a, "-O2") || !strcmp(a, "-O3")) co.opt_level = a[2] - '0';
        else if (!strcmp(a, "--release")) co.overflow_saturate = true;
        else if (!strcmp(a, "--overflow=trap")) co.overflow_saturate = false;
        else if (!strcmp(a, "--overflow=saturate")) co.overflow_saturate = true;
        else if (!strncmp(a, "--emit=", 7)) emit = a + 7;
        else if (!strcmp(a, "--format=agent-json")) json = true;
        else if (!strcmp(a, "--format=human")) json = false;
        else if (!strncmp(a, "--std=", 6)) std_dir = a + 6;
        else if (!strncmp(a, "--cpu=", 6)) co.cpu = a + 6;
        else if (!strncmp(a, "--link=", 7)) vec_push(link_inputs, (char *)a + 7);
        else if (!strncmp(a, "--emit-header=", 14)) header_path = a + 14;
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
    if (!input) {
        diag_emit_fatal(json, "E9001", "no input file");
        return 2;
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
    SemaOptions so = {.std_dir = std_dir ? std_dir : find_std(argv[0])};
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
    char objpath[PATH_MAX];
    if (final_link) {
        snprintf(objpath, sizeof objpath, "%s.tmp%d.o", out, (int)getpid());
        co.out_path = objpath;
    } else {
        co.out_path = out;
    }
    if (!codegen_program(&co)) {
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
            // Programs that use the Phalange host ABI (std/io, std/net) get the
            // native implementation of it linked in.
            bool uses_host_abi = false;
            for (int i = 0; i < g_prog.host_fns.len; i++)
                if (g_prog.host_fns.data[i]->host_module && !strcmp(g_prog.host_fns.data[i]->host_module, "phalange"))
                    uses_host_abi = true;
            if (uses_host_abi || g_needs_native_runtime) {
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
    diag_emit(json, json ? stdout : stderr);
    return 0;
}
