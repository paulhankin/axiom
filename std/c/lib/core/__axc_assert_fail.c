/* __axc_assert_fail (<assert.h>): the message of an assertion that failed
   (7.2.1.1p2): "file:line: function: assertion failed: expression", on the
   standard error stream (unbuffered), then abort. */
#include <assert.h>
#include <stdlib.h>
#include "axc/core_impl.h"

static size_t length(const char *s) {
    size_t n = 0;
    while (s[n])
        n++;
    return n;
}

static void put(const char *s) {
    __axc_write_err(s, length(s));
}

void __axc_assert_fail(const char *expr, const char *file, long line, const char *func) {
    char num[24];
    char *p = num + sizeof num;
    unsigned long v = line < 0 ? 0 : (unsigned long)line;
    *--p = 0;
    do {
        *--p = (char)('0' + v % 10);
        v /= 10;
    } while (v);
    put(file);
    put(":");
    put(p);
    put(": ");
    if (func) {
        put(func);
        put(": ");
    }
    put("assertion failed: ");
    put(expr);
    put("\n");
    abort();
}
