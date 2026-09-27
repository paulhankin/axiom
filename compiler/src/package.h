// package.h - packages: phalange.toml manifests, dependencies, pkg: imports.
#ifndef PH_PACKAGE_H
#define PH_PACKAGE_H

#include "util.h"

typedef struct Package Package;
typedef Vec(Package *) PackageVec;
typedef Vec(char *) StrVec;

struct Package {
    Sym name;
    const char *version;
    const char *dir;       // real path of the package root (where phalange.toml is)
    const char *entry;     // real path of the entry module
    int manifest_file;     // g_files index, for diagnostics
    SymMap deps;           // dependency name -> Package*
    Vec(Sym) dep_names;
    Vec(Span) dep_spans;
};

// The package containing `file_path` (nearest phalange.toml above it), or NULL.
Package *pkg_for_file(const char *file_path);
// Loads the package rooted at `dir` (which must contain phalange.toml).
Package *pkg_load_dir(const char *dir, Span *from);
// Resolves `import "pkg:name[/sub/path]"` made from `importer` (a module's
// package, may be NULL). Writes the module's file path; false on error
// (reported at `sp`).
bool pkg_resolve_import(Package *importer, const char *spec, Span sp, char *out, size_t cap);
// Creates a new package skeleton in ./name. Returns false on error.
bool pkg_new(const char *name, bool json);
// Every .ph file under the package's src/ directory (sorted).
void pkg_source_files(Package *p, StrVec *out);

// The toolchain cache: $PHALANGE_CACHE, $XDG_CACHE_HOME/phalange or ~/.cache/phalange.
const char *pkg_cache_dir(void);

#endif
