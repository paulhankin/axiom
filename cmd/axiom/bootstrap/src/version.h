// version.h - the compiler's version (docs/releasing.md); the same as
// VERSION in cmd/axiom/src/version.ax.
#ifndef AXIOM_VERSION_H
#define AXIOM_VERSION_H

#include <stdbool.h>
#include <stdint.h>

#define AXIOM_VERSION "0.1.0"

// The numbers of a version "a", "a.b" or "a.b.c" (missing ones are 0), or
// false if it is not one.
static inline bool version_parse(const char *v, uint32_t out[3]) {
    int part = 0, digits = 0;
    out[0] = out[1] = out[2] = 0;
    for (; *v; v++) {
        if (*v == '.') {
            if (digits == 0 || part == 2) return false;
            part++;
            digits = 0;
        } else if (*v >= '0' && *v <= '9' && digits < 6) {
            out[part] = out[part] * 10 + (uint32_t)(*v - '0');
            digits++;
        } else {
            return false;
        }
    }
    return digits > 0;
}

// Whether this compiler is at least version `need`.
static inline bool version_at_least(const uint32_t need[3]) {
    uint32_t have[3];
    version_parse(AXIOM_VERSION, have);
    for (int i = 0; i < 3; i++)
        if (have[i] != need[i]) return have[i] > need[i];
    return true;
}

#endif
