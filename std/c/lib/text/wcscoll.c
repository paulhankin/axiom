/* wcscoll (C17 7.29.4.4.2): every locale collates as "C" does, by the
   characters' code points: strcoll's order of their UTF-8 bytes (or bytes,
   in the single-byte locale), so that the two agree on a string and its
   conversion. Where wchar_t is UTF-16 (Windows) that is not the order of
   the code units: a surrogate (U+D800-U+DFFF, half of a character beyond
   U+FFFF) comes after U+E000-U+FFFF (__axc_wcs_key). */
#include <axc/text.h>
#include <wchar.h>

int wcscoll(const wchar_t *s1, const wchar_t *s2) {
#if __AXIOM_WCHAR_WIDTH__ == 16
    for (; *s1 == *s2; s1++, s2++)
        if (*s1 == 0)
            return 0;
    return __axc_wcs_key(*s1) < __axc_wcs_key(*s2) ? -1 : 1;
#else
    return wcscmp(s1, s2);
#endif
}
