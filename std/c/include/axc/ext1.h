/* axc/ext1.h - whether a translation unit asked for Annex K's interfaces
   (C11 K.3.1.1, C17 K.3.1.1, C23 K.3.1.1). Each header of K.3 (errno.h,
   stddef.h, stdint.h, stdio.h, stdlib.h, string.h, time.h, wchar.h)
   includes this one first, before its own include guard, every time it is
   included. Private to the compiler's headers.

   - __STDC_WANT_LIB_EXT1__ defined as 1: the headers declare Annex K's
     names (K.3.1.1p3); defined as 0: they do not (p2). Not defined: they
     do not (the implementation's choice, p3: the names stay the program's).
     Another definition is an error.
   - p4: the macro shall be defined identically (here: undefined, or of the
     same value) for every inclusion of these headers in the translation
     unit; an inclusion that sees it otherwise than the first is an error,
     as an #error directive gives one.

   After it, __AXC_EXT1 is 1 if the first inclusion asked for the names,
   else 0. */

#undef __AXC_EXT1_NOW
#if !defined(__STDC_WANT_LIB_EXT1__)
#define __AXC_EXT1_NOW 2
#elif (0 - __STDC_WANT_LIB_EXT1__ - 1) == 1
/* (defined with no value: 0 - - 1) */
#error "__STDC_WANT_LIB_EXT1__ must be defined as 0 or 1 (K.3.1.1)"
#define __AXC_EXT1_NOW 2
#elif __STDC_WANT_LIB_EXT1__ == 1
#define __AXC_EXT1_NOW 1
#elif __STDC_WANT_LIB_EXT1__ == 0
#define __AXC_EXT1_NOW 0
#else
#error "__STDC_WANT_LIB_EXT1__ must be defined as 0 or 1 (K.3.1.1)"
#define __AXC_EXT1_NOW 2
#endif

#if !defined(__AXC_EXT1_FIRST)
#if __AXC_EXT1_NOW == 1
#define __AXC_EXT1_FIRST 1
#define __AXC_EXT1 1
#elif __AXC_EXT1_NOW == 0
#define __AXC_EXT1_FIRST 0
#define __AXC_EXT1 0
#else
#define __AXC_EXT1_FIRST 2
#define __AXC_EXT1 0
#endif
#elif __AXC_EXT1_FIRST != __AXC_EXT1_NOW
#error "__STDC_WANT_LIB_EXT1__ is not defined identically for every inclusion of the headers of Annex K (K.3.1.1p4)"
#endif
#undef __AXC_EXT1_NOW
