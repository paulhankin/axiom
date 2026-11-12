/* mbrtoc32 (C17 7.28.1.3): as mbrtowc, to UTF-32. */
#include <axc/text.h>
#include <uchar.h>

size_t mbrtoc32(char32_t *restrict pc32, const char *restrict s, size_t n, mbstate_t *restrict ps) {
    static mbstate_t internal;
    unsigned long c;
    size_t r;
    if (!ps)
        ps = &internal;
    if (!s) {
        pc32 = 0;
        s = "";
        n = 1;
    }
    r = __axc_mb_get(&c, s, n, ps);
    if (r != (size_t)-1 && r != (size_t)-2 && pc32)
        *pc32 = (char32_t)c;
    return r;
}
