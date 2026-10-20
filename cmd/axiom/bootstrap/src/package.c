// package.c - packages (spec §4.6).
//
// A package is a directory with an axiom.toml manifest:
//
//     [package]
//     name = "geometry"
//     version = "0.1.0"
//     axiom = "0.1"                 # optional: the oldest compiler that builds it
//     entry = "src/lib.ax"          # optional: default src/lib.ax, else src/main.ax
//
//     [dependencies]
//     vecmath = { path = "../vecmath" }
//     json = { git = "https://example.com/json.git", rev = "<full commit hash>" }
//
// Modules import dependencies with "pkg:name" (the entry module) or
// "pkg:name/sub/mod" (src/sub/mod.ax). Only a package's own dependencies are
// visible to it, and each package name resolves to one source per build.
#include "package.h"

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "diag.h"
#include "version.h"

extern char **environ;

static SymMap pkg_by_dir;   // real dir -> Package*
static SymMap pkg_by_name;  // name -> Package* (one source per name per build)

static bool is_dir(const char *p) {
    struct stat st;
    return stat(p, &st) == 0 && S_ISDIR(st.st_mode);
}
static bool is_file(const char *p) {
    struct stat st;
    return stat(p, &st) == 0 && S_ISREG(st.st_mode);
}

// ---------------------------------------------------------------------------
// Manifest parsing: a TOML subset (sections, string values, inline tables).
// ---------------------------------------------------------------------------
typedef struct Parser {
    SourceFile *f;
    uint32_t pos;
    bool failed;
} Parser;

static Span pspan(Parser *P, uint32_t a, uint32_t b) { return (Span){P->f->id, a, b}; }

static void perr(Parser *P, uint32_t a, uint32_t b, const char *msg) {
    diag_error(pspan(P, a, b), "E0215", "invalid axiom.toml: %s", msg);
    P->failed = true;
}

static void skip_ws(Parser *P) {
    while (P->pos < P->f->len && (P->f->text[P->pos] == ' ' || P->f->text[P->pos] == '\t')) P->pos++;
    if (P->pos < P->f->len && P->f->text[P->pos] == '#')
        while (P->pos < P->f->len && P->f->text[P->pos] != '\n') P->pos++;
}

static bool at_eol(Parser *P) {
    skip_ws(P);
    return P->pos >= P->f->len || P->f->text[P->pos] == '\n' || P->f->text[P->pos] == '\r';
}

static const char *parse_key(Parser *P, uint32_t *start) {
    skip_ws(P);
    *start = P->pos;
    while (P->pos < P->f->len) {
        char c = P->f->text[P->pos];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '-')) break;
        P->pos++;
    }
    if (P->pos == *start) return NULL;
    return arena_strndup(P->f->text + *start, P->pos - *start);
}

static const char *parse_string(Parser *P) {
    skip_ws(P);
    if (P->pos >= P->f->len || P->f->text[P->pos] != '"') {
        perr(P, P->pos, P->pos + 1, "expected a \"string\"");
        return NULL;
    }
    uint32_t a = ++P->pos;
    while (P->pos < P->f->len && P->f->text[P->pos] != '"' && P->f->text[P->pos] != '\n') P->pos++;
    if (P->pos >= P->f->len || P->f->text[P->pos] != '"') {
        perr(P, a - 1, P->pos, "unterminated string");
        return NULL;
    }
    const char *s = arena_strndup(P->f->text + a, P->pos - a);
    P->pos++;
    return s;
}

static bool expect_ch(Parser *P, char c) {
    skip_ws(P);
    if (P->pos < P->f->len && P->f->text[P->pos] == c) {
        P->pos++;
        return true;
    }
    perr(P, P->pos, P->pos + 1, arena_printf("expected '%c'", c));
    return false;
}

static void next_line(Parser *P) {
    while (P->pos < P->f->len && P->f->text[P->pos] != '\n') P->pos++;
    if (P->pos < P->f->len) P->pos++;
}

typedef struct DepSpec {
    Sym name;
    Span span;
    const char *path, *git, *rev;
} DepSpec;

static bool valid_name(const char *s) {
    if (!*s) return false;
    for (const char *p = s; *p; p++)
        if (!(islower((unsigned char)*p) || isdigit((unsigned char)*p) || *p == '_')) return false;
    return !isdigit((unsigned char)s[0]);
}

// ---------------------------------------------------------------------------
// Git dependencies: cloned once per (url, rev) into the cache.
// ---------------------------------------------------------------------------
static int run_quiet(char *const argv[]) {
    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_addopen(&fa, 1, "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_addopen(&fa, 2, "/dev/null", O_WRONLY, 0);
    pid_t pid;
    int rc = posix_spawnp(&pid, argv[0], &fa, NULL, argv, environ);
    posix_spawn_file_actions_destroy(&fa);
    if (rc != 0) return -1;
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

const char *pkg_cache_dir(void) {
    const char *c = getenv("AXIOM_CACHE");
    if (c && *c) return c;
    const char *x = getenv("XDG_CACHE_HOME");
    if (x && *x) return arena_printf("%s/axiom", x);
    const char *h = getenv("HOME");
    if (h && *h) return arena_printf("%s/.cache/axiom", h);
    // No home: a directory of this run's own, since a shared one in /tmp
    // could hold another user's objects and checkouts.
    static const char *run_cache;
    if (!run_cache) {
        const char *t = getenv("TMPDIR");
        char *d = (char *)arena_printf("%s/axiom-cache-XXXXXX", t && *t ? t : "/tmp");
        if (!mkdtemp(d)) {
            fprintf(stderr, "axiom: no cache directory: set AXIOM_CACHE or HOME\n");
            exit(2);
        }
        run_cache = d;
    }
    return run_cache;
}

static void mkdirs(const char *path) {
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

// Is s a full commit hash (SHA-1 or SHA-256)?
static bool is_commit(const char *s) {
    size_t n = strlen(s);
    if (n != 40 && n != 64) return false;
    for (size_t i = 0; i < n; i++)
        if (!isdigit((unsigned char)s[i]) && !(s[i] >= 'a' && s[i] <= 'f')) return false;
    return true;
}

static const char *fetch_git(const char *url, const char *rev, Span sp) {
    if (!is_commit(rev) || url[0] == '-') return NULL;  // (reported with the manifest)
    char key[256];
    int k = 0;
    for (const char *p = url; *p && k < 150; p++) key[k++] = isalnum((unsigned char)*p) ? *p : '_';
    key[k] = 0;
    const char *dir = arena_printf("%s/git/%s-%s", pkg_cache_dir(), key, rev);
    if (is_file(arena_printf("%s/axiom.toml", dir))) return dir;  // cached
    mkdirs(arena_printf("%s/git", pkg_cache_dir()));
    const char *tmp = arena_printf("%s.tmp%d", dir, (int)getpid());
    char *clone[] = {"git", "clone", "--quiet", "--", (char *)url, (char *)tmp, NULL};
    char *co[] = {"git", "-C", (char *)tmp, "-c", "advice.detachedHead=false", "checkout", "--quiet", (char *)rev, NULL};
    if (run_quiet(clone) != 0 || run_quiet(co) != 0) {
        char *rm[] = {"rm", "-rf", (char *)tmp, NULL};
        run_quiet(rm);
        diag_error(sp, "E0216", "cannot fetch %s at %s (is the URL reachable and the commit correct?)", url, rev);
        return NULL;
    }
    if (rename(tmp, dir) != 0) {  // lost a race with another build: use theirs
        char *rm[] = {"rm", "-rf", (char *)tmp, NULL};
        run_quiet(rm);
    }
    return dir;
}

// ---------------------------------------------------------------------------
// Loading
// ---------------------------------------------------------------------------
Package *pkg_load_dir(const char *dir_in, Span *from) {
    char dir[PATH_MAX];
    if (!realpath(dir_in, dir)) {
        if (from) diag_error(*from, "E0217", "package directory '%s' does not exist", dir_in);
        return NULL;
    }
    Sym key = intern_cstr(dir);
    Package *p = symmap_get(&pkg_by_dir, key);
    if (p) return p;
    const char *mpath = arena_printf("%s/axiom.toml", dir);
    const char *display = mpath;
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof cwd) && !strncmp(mpath, cwd, strlen(cwd)) && mpath[strlen(cwd)] == '/')
        display = mpath + strlen(cwd) + 1;
    SourceFile *f = load_file(mpath, arena_strndup(display, strlen(display)));
    if (!f) {
        if (from) diag_error(*from, "E0217", "no axiom.toml in '%s'", dir_in);
        return NULL;
    }
    p = NEW(Package);
    p->dir = key;
    p->manifest_file = f->id;
    symmap_put(&pkg_by_dir, key, p);

    Parser P = {.f = f};
    const char *section = "";
    const char *entry = NULL;
    Vec(DepSpec) specs = {0};
    while (P.pos < f->len) {
        skip_ws(&P);
        if (at_eol(&P)) {
            next_line(&P);
            continue;
        }
        if (f->text[P.pos] == '[') {
            uint32_t a = P.pos++;
            uint32_t ks;
            const char *name = parse_key(&P, &ks);
            if (!name || !expect_ch(&P, ']')) {
                next_line(&P);
                continue;
            }
            if (strcmp(name, "package") && strcmp(name, "dependencies"))
                perr(&P, a, P.pos, arena_printf("unknown section [%s]", name));
            section = name;
            next_line(&P);
            continue;
        }
        uint32_t ks;
        const char *key_s = parse_key(&P, &ks);
        if (!key_s) {
            perr(&P, P.pos, P.pos + 1, "expected key = value");
            next_line(&P);
            continue;
        }
        uint32_t ke = P.pos;
        if (!expect_ch(&P, '=')) {
            next_line(&P);
            continue;
        }
        if (!strcmp(section, "package")) {
            const char *v = parse_string(&P);
            if (!v) {
                next_line(&P);
                continue;
            }
            if (!strcmp(key_s, "name")) {
                if (!valid_name(v)) perr(&P, ks, P.pos, "package names use lower-case letters, digits and '_'");
                p->name = intern_cstr(v);
            } else if (!strcmp(key_s, "version")) {
                p->version = v;
            } else if (!strcmp(key_s, "axiom")) {
                uint32_t need[3];
                if (!version_parse(v, need))
                    perr(&P, ks, P.pos, "axiom = \"...\" must be a version, such as \"0.1\"");
                else if (!version_at_least(need))
                    diag_error(pspan(&P, ks, P.pos), "E0220", "this package needs axiom %s or later; this compiler is %s", v,
                               AXIOM_VERSION);
            } else if (!strcmp(key_s, "entry")) {
                entry = v;
            } else {
                perr(&P, ks, ke, arena_printf("unknown key '%s' in [package]", key_s));
            }
        } else if (!strcmp(section, "dependencies")) {
            DepSpec d = {intern_cstr(key_s), pspan(&P, ks, ke), NULL, NULL, NULL};
            if (!valid_name(key_s)) perr(&P, ks, ke, "package names use lower-case letters, digits and '_'");
            if (!expect_ch(&P, '{')) {
                next_line(&P);
                continue;
            }
            for (;;) {
                skip_ws(&P);
                if (P.pos < f->len && f->text[P.pos] == '}') {
                    P.pos++;
                    break;
                }
                uint32_t fs;
                const char *fk = parse_key(&P, &fs);
                if (!fk || !expect_ch(&P, '=')) break;
                const char *fv = parse_string(&P);
                if (!fv) break;
                if (!strcmp(fk, "path")) d.path = fv;
                else if (!strcmp(fk, "git")) d.git = fv;
                else if (!strcmp(fk, "rev")) d.rev = fv;
                else perr(&P, fs, P.pos, arena_printf("unknown dependency field '%s' (path, git, rev)", fk));
                skip_ws(&P);
                if (P.pos < f->len && f->text[P.pos] == ',') P.pos++;
            }
            if (!d.path && !d.git) perr(&P, ks, ke, "a dependency needs path = \"...\" or git = \"...\"");
            if (d.git && !d.rev)
                perr(&P, ks, ke, "git dependencies must pin a commit: rev = \"<full commit hash>\"");
            else if (d.git && !is_commit(d.rev))  // (a cache directory's name; git takes ':/text' too)
                perr(&P, ks, ke, "rev must be a full commit hash: 40 or 64 lower-case hex digits");
            if (d.git && d.git[0] == '-') perr(&P, ks, ke, "a git URL cannot start with '-'");
            for (int i = 0; i < specs.len; i++)
                if (specs.data[i].name == d.name) perr(&P, ks, ke, "duplicate dependency");
            vec_push(specs, d);
        } else {
            perr(&P, ks, ke, "key outside of [package] or [dependencies]");
        }
        if (!at_eol(&P)) perr(&P, P.pos, P.pos + 1, "unexpected text after value");
        next_line(&P);
    }
    if (!p->name) {
        diag_error((Span){f->id, 0, 0}, "E0215", "invalid axiom.toml: [package] needs name = \"...\"");
        p->name = intern_cstr("<unnamed>");
    }
    // one source per package name
    Package *same = symmap_get(&pkg_by_name, p->name);
    if (same && same != p) {
        Diag *dg = diag_error(from ? *from : (Span){f->id, 0, 0}, "E0213",
                              "two different packages are both named '%s'; a build can use only one", p->name);
        diag_note(dg, (Span){same->manifest_file, 0, 0}, "one is here");
        diag_note(dg, (Span){f->id, 0, 0}, "the other is here");
    } else {
        symmap_put(&pkg_by_name, p->name, p);
    }
    // entry module
    if (entry) p->entry = arena_printf("%s/%s", dir, entry);
    else if (is_file(arena_printf("%s/src/lib.ax", dir))) p->entry = arena_printf("%s/src/lib.ax", dir);
    else p->entry = arena_printf("%s/src/main.ax", dir);
    // dependencies (sources resolved now; loaded as imports happen)
    for (int i = 0; i < specs.len; i++) {
        DepSpec *d = &specs.data[i];
        if (!d->path && !d->rev) continue;
        const char *ddir = d->path ? (d->path[0] == '/' ? d->path : arena_printf("%s/%s", dir, d->path))
                                   : fetch_git(d->git, d->rev, d->span);
        if (!ddir) continue;
        Package *dp = pkg_load_dir(ddir, &d->span);
        if (!dp) continue;
        if (dp->name != d->name) {
            diag_error(d->span, "E0215", "dependency '%s' points at package '%s'", d->name, dp->name);
            continue;
        }
        symmap_put(&p->deps, d->name, dp);
        vec_push(p->dep_names, d->name);
        vec_push(p->dep_spans, d->span);
    }
    free(specs.data);
    return p;
}

// The package containing file_path (nearest axiom.toml above it), or NULL.
// The search stops at a directory anyone may write to.
Package *pkg_for_file(const char *file_path) {
    char dir[PATH_MAX];
    if (!realpath(file_path, dir)) return NULL;
    for (;;) {
        char *slash = strrchr(dir, '/');
        if (!slash) return NULL;
        *slash = 0;
        if (!dir[0]) return NULL;
        Package *p = symmap_get(&pkg_by_dir, intern_cstr(dir));
        if (p) return p;
        // (a manifest in a directory anyone may write to, like /tmp, could be anyone's)
        struct stat st;
        if (stat(dir, &st) != 0 || (st.st_mode & S_IWOTH)) return NULL;
        if (is_file(arena_printf("%s/axiom.toml", dir))) return pkg_load_dir(dir, NULL);
    }
}

bool pkg_resolve_import(Package *importer, const char *spec, Span sp, char *out, size_t cap) {
    const char *rest = spec + 4;  // after "pkg:"
    const char *slash = strchr(rest, '/');
    Sym name = slash ? intern(rest, (size_t)(slash - rest)) : intern_cstr(rest);
    if (!importer) {
        diag_error(sp, "E0214", "'%s' imports package '%s', but this file is not part of a package (no axiom.toml)",
                   spec, name);
        return false;
    }
    Package *dep = importer->name == name ? importer : symmap_get(&importer->deps, name);
    if (!dep) {
        Diag *d = diag_error(sp, "E0214", "package '%s' is not a dependency of '%s'", name, importer->name);
        diag_note(d, (Span){importer->manifest_file, 0, 0}, "add it under [dependencies] in this manifest");
        return false;
    }
    if (slash) {
        const char *sub = slash + 1;
        size_t n = strlen(sub);
        bool ext = n > 3 && !strcmp(sub + n - 3, ".ax");
        snprintf(out, cap, "%s/src/%s%s", dep->dir, sub, ext ? "" : ".ax");
    } else {
        snprintf(out, cap, "%s", dep->entry);
    }
    return true;
}

static int cmp_str(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }

static void collect(const char *dir, StrVec *out) {
    DIR *d = opendir(dir);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (e->d_name[0] == '.') continue;
        char *p = arena_printf("%s/%s", dir, e->d_name);
        size_t n = strlen(e->d_name);
        if (is_dir(p)) collect(p, out);
        else if (n > 3 && !strcmp(e->d_name + n - 3, ".ax")) vec_push(*out, p);
    }
    closedir(d);
}

void pkg_source_files(Package *p, StrVec *out) {
    int start = out->len;
    collect(arena_printf("%s/src", p->dir), out);
    qsort(out->data + start, (size_t)(out->len - start), sizeof(char *), cmp_str);
}

bool pkg_new(const char *name, bool json) {
    if (!valid_name(name)) {
        diag_emit_fatal(json, "E0215", "package names use lower-case letters, digits and '_'");
        return false;
    }
    if (is_dir(name) || is_file(name)) {
        diag_emit_fatal(json, "E9005", arena_printf("'%s' already exists", name));
        return false;
    }
    mkdirs(arena_printf("%s/src", name));
    FILE *m = fopen(arena_printf("%s/axiom.toml", name), "w");
    FILE *s = fopen(arena_printf("%s/src/lib.ax", name), "w");
    if (!m || !s) {
        diag_emit_fatal(json, "E9005", "cannot create the package files");
        return false;
    }
    uint32_t have[3];
    version_parse(AXIOM_VERSION, have);
    fprintf(m,
            "[package]\nname = \"%s\"\nversion = \"0.1.0\"\naxiom = \"%u.%u\"\n\n[dependencies]\n# other = { path = \"../other\" }\n",
            name, have[0], have[1]);
    fprintf(s,
            "// %s: library entry module (import it as \"pkg:%s\").\n\n"
            "pub fn add(a: i32, b: i32) i32 {\n    return a + b;\n}\n\n"
            "test \"add\" {\n    @expect_eq(add(2, 3), 5);\n}\n",
            name, name);
    fclose(m);
    fclose(s);
    if (json) printf("{\"status\":\"ok\",\"created\":\"%s\"}\n", name);
    else printf("created package '%s': %s/axiom.toml, %s/src/lib.ax\n", name, name, name);
    return true;
}
