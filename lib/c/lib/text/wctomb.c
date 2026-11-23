/* wctomb (C17 7.22.7.3). There are no shift states: with s a null pointer
   it returns 0 (and forgets a kept surrogate). Where wchar_t has 16 bits a
   high surrogate is kept (0 bytes stored) until its low one comes. */
#include <axc/text.h>
#include <wchar.h>

int wctomb(char *s, wchar_t wc) {
    static _Thread_local mbstate_t internal; /* (each thread's: free of data races) */
    size_t r;
    if (!s) {
        mbstate_t initial = {0};
        internal = initial;
        return 0;
    }
    r = wcrtomb(s, wc, &internal);
    return r == (size_t)-1 ? -1 : (int)r;
}
