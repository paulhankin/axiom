/* getenv_s (C11 K.3.6.2.1): the value of the environment variable name:
   its length into *len (if len is not null; 0 if there is none) and, if
   it is shorter than maxsize, the string into value (value[0] the null
   character when there is none and maxsize is not 0). Zero if the value
   was found and stored, else nonzero (ENOENT: not found; ERANGE: too long
   for value). Runtime-constraints: name not null, maxsize at most
   RSIZE_MAX, maxsize 0 when value is null; after a violation *len is 0
   (if len is not null) and value[0] the null character (if value is not
   null and maxsize is 1 to RSIZE_MAX). getenv's thread-safety holds
   (std/c/include/stdlib.h). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <string.h>

errno_t getenv_s(size_t *__AXC_RESTRICT len, char *__AXC_RESTRICT value, rsize_t maxsize,
                 const char *__AXC_RESTRICT name) {
    const char *what = NULL, *v;
    errno_t e = EINVAL;
    size_t n;
    if (name == NULL)
        what = "name is a null pointer";
    else if (maxsize > RSIZE_MAX)
        what = "maxsize is greater than RSIZE_MAX", e = ERANGE;
    else if (value == NULL && maxsize != 0)
        what = "value is a null pointer and maxsize is not zero";
    if (what != NULL) {
        if (len != NULL)
            *len = 0;
        if (value != NULL && maxsize > 0 && maxsize <= RSIZE_MAX)
            value[0] = 0;
        __axc_ext1_violation("getenv_s", what, e);
        return e;
    }
    v = getenv(name);
    if (v == NULL) {
        if (len != NULL)
            *len = 0;
        if (maxsize > 0)
            value[0] = 0;
        return ENOENT;
    }
    n = strlen(v);
    if (len != NULL)
        *len = n;
    if (n >= maxsize)
        return ERANGE;
    memcpy(value, v, n + 1);
    return 0;
}
