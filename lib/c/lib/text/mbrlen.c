/* mbrlen (C17 7.29.6.3.1): mbrtowc with no character stored, with its own
   internal state. */
#include <wchar.h>

size_t mbrlen(const char *restrict s, size_t n, mbstate_t *restrict ps) {
    static _Thread_local mbstate_t internal; /* (each thread's: free of data races) */
    return mbrtowc(0, s, n, ps ? ps : &internal);
}
