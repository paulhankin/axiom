// units.c - separate compilation (spec §16.7).
//
// Each module is compiled to its own object file, cached under
// <cache>/obj/<key>.o, where the key hashes everything the object depends on:
// the compiler binary, the code generation options, the module's source and
// path, and (recursively) the keys of the modules it imports. A build
// compiles only the modules whose key has no cached object, in parallel
// worker processes forked after type checking, then generates the small
// per-program link unit (globals, error codes, runtime state, exports).
#include "units.h"

#include <limits.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "diag.h"
#include "package.h"

// Two independent 64-bit hashes make a 128-bit key.
typedef struct Hasher {
    uint64_t a, b;
} Hasher;

static void hash_bytes(Hasher *h, const void *p, size_t n) {
    const unsigned char *s = p;
    for (size_t i = 0; i < n; i++) {
        h->a = (h->a ^ s[i]) * 0x100000001b3ull;
        h->b = (h->b + s[i] + 1) * 0x9E3779B97F4A7C15ull;
        h->b ^= h->b >> 29;
    }
}

static void hash_str(Hasher *h, const char *s) {
    hash_bytes(h, s ? s : "", s ? strlen(s) + 1 : 1);  // with the NUL, as a separator
}

static Hasher hasher(void) { return (Hasher){0xcbf29ce484222325ull, 0x2545F4914F6CDD1Dull}; }

static const char *hex_key(Hasher *h) { return arena_printf("%016llx%016llx", (unsigned long long)h->a, (unsigned long long)h->b); }

// The compiler's own identity: a hash of its executable.
static const char *compiler_id(void) {
    static const char *id;
    if (id) return id;
    Hasher h = hasher();
    size_t len = 0;
    char *bytes = read_entire_file("/proc/self/exe", &len);
    if (bytes) {
        hash_bytes(&h, bytes, len);
    } else {
        hash_str(&h, __DATE__ " " __TIME__);
    }
    id = hex_key(&h);
    return id;
}

static const char *options_id(CgOptions *o) {
    return arena_printf("unit-v1|%s|%d|%d|%d|%d|%d|%d|%d|%d|%s", g_target.triple, g_target.ptr_bits, o->opt_level,
                        o->overflow_saturate, o->stack_traces, o->profile, o->sanitize_address, g_test_mode, o->emit,
                        o->cpu ? o->cpu : "");
}

typedef struct Unit {
    Module *m;
    const char *key;
    const char *obj;
    bool cached;
} Unit;

static SymMap key_memo;

static const char *module_key(Module *m, const char *opts) {
    Sym mk = intern_cstr(m->path);
    const char *k = symmap_get(&key_memo, mk);
    if (k) return k;
    Hasher h = hasher();
    hash_str(&h, compiler_id());
    hash_str(&h, opts);
    SourceFile *f = g_files.data[m->file->id];
    hash_str(&h, f->display);
    hash_bytes(&h, f->text, f->len);
    bool has_tests = false;
    for (int i = 0; i < g_prog.tests.len; i++)
        if (g_prog.tests.data[i]->module == m) has_tests = true;
    hash_str(&h, has_tests ? "tests" : "no-tests");
    for (int i = 0; i < m->decls.len; i++) {
        Decl *d = m->decls.data[i];
        if (d->kind == D_IMPORT && d->imported) hash_str(&h, module_key(d->imported, opts));
    }
    k = hex_key(&h);
    symmap_put(&key_memo, mk, (void *)k);
    return k;
}

// On wasm every thread-local of the program lives in one block whose layout
// each module's code bakes in, so a module's object also depends on the
// thread-locals of modules it does not import: their order, sizes and
// alignments. NULL when there is no such dependency.
static const char *tls_layout_id(void) {
    if (!g_target.wasm) return NULL;
    Hasher h = hasher();
    bool any = false;
    for (int i = 0; i < g_prog.globals.len; i++) {
        Decl *d = g_prog.globals.data[i];
        if (!d->tls) continue;
        any = true;
        hash_str(&h, d->module->path);
        hash_str(&h, d->name);
        hash_str(&h, arena_printf("%lld/%lld", (long long)type_size(d->type), (long long)type_align(d->type)));
    }
    return any ? hex_key(&h) : NULL;
}

static bool file_ok(const char *p) {
    struct stat st;
    return stat(p, &st) == 0 && S_ISREG(st.st_mode) && st.st_size > 0;
}

static void make_dirs(const char *path) {
    char buf[PATH_MAX];
    snprintf(buf, sizeof buf, "%s", path);
    for (char *p = buf + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            mkdir(buf, 0755);
            *p = '/';
        }
    }
    mkdir(buf, 0755);
}

// Compiles one module in a child process. Returns its pid (or -1).
static pid_t spawn_unit(CgOptions *base, Unit *u) {
    fflush(stdout);
    fflush(stderr);
    pid_t pid = fork();
    if (pid != 0) return pid;
    CgOptions co = *base;
    co.unit = CG_MODULE;
    co.module = u->m;
    const char *tmp = arena_printf("%s.tmp%d", u->obj, (int)getpid());
    co.out_path = tmp;
    bool ok = codegen_program(&co);
    if (!ok) {
        diag_emit(false, stderr);
        unlink(tmp);
        _exit(1);
    }
    if (rename(tmp, u->obj) != 0) {
        unlink(tmp);
        _exit(1);
    }
    _exit(0);
}

bool build_units(CgOptions *base, const char *link_obj, UnitObjs *objs, UnitStats *stats) {
    const char *opts = options_id(base);
    const char *dir = arena_printf("%s/obj", pkg_cache_dir());
    make_dirs(dir);
    int n = g_prog.modules.len;
    Unit *units = NEWN(Unit, n + 1);
    Vec(Unit *) todo = {0};
    const char *tls = tls_layout_id();
    for (int i = 0; i < n; i++) {
        Unit *u = &units[i];
        u->m = g_prog.modules.data[i];
        u->key = module_key(u->m, opts);
        if (tls) {
            Hasher h = hasher();
            hash_str(&h, u->key);
            hash_str(&h, tls);
            u->key = hex_key(&h);
        }
        u->obj = arena_printf("%s/%s.o", dir, u->key);
        u->cached = file_ok(u->obj);
        if (!u->cached) vec_push(todo, u);
    }
    // Compile what is missing, a few at a time.
    long cpus = sysconf(_SC_NPROCESSORS_ONLN);
    int width = cpus > 0 ? (int)cpus : 1;
    if (width > 16) width = 16;
    int running = 0, next = 0;
    bool failed = false;
    pid_t *pids = NEWN(pid_t, todo.len + 1);
    while (next < todo.len || running > 0) {
        while (!failed && running < width && next < todo.len) {
            pid_t p = spawn_unit(base, todo.data[next]);
            if (p < 0) {
                failed = true;
                break;
            }
            pids[next++] = p;
            running++;
        }
        if (running == 0) break;
        int status;
        pid_t done = wait(&status);
        if (done < 0) break;
        running--;
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) failed = true;
    }
    if (stats) {
        stats->total = n;
        stats->compiled = todo.len;
    }
    free(todo.data);
    if (failed) return false;
    for (int i = 0; i < n; i++) vec_push(*objs, (char *)units[i].obj);
    // The link unit, in this process.
    CgOptions co = *base;
    co.unit = CG_LINK;
    co.module = NULL;
    co.out_path = link_obj;
    return codegen_program(&co);
}

// The toolchain's own C runtime (runtime/*.c) is compiled once per content
// and compiler, like modules. Returns the object, or NULL to compile it
// in the link step instead.
const char *cached_runtime_object(const char *cc, const char *src) {
    size_t len = 0;
    char *text = read_entire_file(src, &len);
    if (!text) return NULL;
    Hasher h = hasher();
    hash_str(&h, "runtime-v1");
    hash_str(&h, cc);
    hash_str(&h, compiler_id());
    hash_bytes(&h, text, len);
    // Its only local header.
    char dir[PATH_MAX];
    snprintf(dir, sizeof dir, "%s", src);
    char *slash = strrchr(dir, '/');
    if (slash) *slash = 0;
    char *hdr = read_entire_file(arena_printf("%s/trap_format.h", slash ? dir : "."), &len);
    if (hdr) {
        hash_bytes(&h, hdr, len);
    }
    const char *cdir = arena_printf("%s/obj", pkg_cache_dir());
    make_dirs(cdir);
    const char *obj = arena_printf("%s/rt-%s.o", cdir, hex_key(&h));
    if (file_ok(obj)) return obj;
    const char *tmp = arena_printf("%s.tmp%d", obj, (int)getpid());
    pid_t pid = fork();
    if (pid == 0) {
        execlp(cc, cc, "-O2", "-c", "-pthread", src, "-o", tmp, (char *)NULL);
        _exit(127);
    }
    int status;
    if (pid < 0 || waitpid(pid, &status, 0) < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0 || rename(tmp, obj) != 0) {
        unlink(tmp);
        return NULL;
    }
    return obj;
}
