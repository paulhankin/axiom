// units.h - separate compilation of modules, with an object cache (spec §16.7).
#ifndef AX_UNITS_H
#define AX_UNITS_H

#include "codegen.h"

typedef Vec(char *) UnitObjs;

typedef struct UnitStats {
    int total, compiled;  // modules in the program; modules compiled (not cached)
    int evicted;          // cached objects removed to keep the cache under its limit
    int64_t evicted_bytes;
} UnitStats;

// A file of the cache directory.
typedef struct CacheEntry {
    const char *path;
    int64_t size, mtime;  // bytes; modification time, nanoseconds since 1970
} CacheEntry;
typedef Vec(CacheEntry) CacheEntries;

// The cache's size limit in bytes: $AXIOM_CACHE_LIMIT, a count of bytes or
// one with a k, m or g suffix (0: no limit); 2 GiB if unset or malformed.
int64_t cache_limit(void);

// The files of the cache directory `dir` (objects, and temporaries left by
// interrupted builds), appended to `out`; their total size.
int64_t cache_entries(const char *dir, CacheEntries *out);

// Compiles every module whose object is not cached, then the link unit into
// `link_obj`. Appends the module objects to `objs`. False on failure (errors
// already reported).
bool build_units(CgOptions *base, const char *link_obj, UnitObjs *objs, UnitStats *stats);

// Object for one of the toolchain's runtime C files, compiled with `cc` and
// cached; NULL if that fails.
const char *cached_runtime_object(const char *cc, const char *src);

#endif
