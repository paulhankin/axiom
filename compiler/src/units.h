// units.h - separate compilation of modules, with an object cache (spec §16.7).
#ifndef PH_UNITS_H
#define PH_UNITS_H

#include "codegen.h"

typedef Vec(char *) UnitObjs;

typedef struct UnitStats {
    int total, compiled;  // modules in the program; modules compiled (not cached)
} UnitStats;

// Compiles every module whose object is not cached, then the link unit into
// `link_obj`. Appends the module objects to `objs`. False on failure (errors
// already reported).
bool build_units(CgOptions *base, const char *link_obj, UnitObjs *objs, UnitStats *stats);

// Object for one of the toolchain's runtime C files, compiled with `cc` and
// cached; NULL if that fails.
const char *cached_runtime_object(const char *cc, const char *src);

#endif
