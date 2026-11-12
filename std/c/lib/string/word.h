/* word.h - what the word-at-a-time functions of std/c/lib/string/ share:
   a machine word (size_t: as wide as a pointer), and the tests of whether
   one of its bytes is zero. Aligned words are read through `const word *`:
   this library is compiled only by this compiler, which does not assume
   that objects of different types do not overlap. */
#ifndef __AXIOM_LIB_WORD_H
#define __AXIOM_LIB_WORD_H

#include <stddef.h>

typedef size_t __axiom_word;

#define __AXIOM_WSIZE sizeof(__axiom_word)
/* the word whose bytes are all 0x01, and all 0x80 */
#define __AXIOM_ONES ((__axiom_word)-1 / 0xFF)
#define __AXIOM_HIGHS (__AXIOM_ONES * 0x80)
/* nonzero if a byte of x is zero (exact: x - ONES borrows into a byte's
   high bit only past a zero byte, and ~x keeps the bytes under 0x80) */
#define __AXIOM_HAS_ZERO(x) (((x) - __AXIOM_ONES) & ~(x) & __AXIOM_HIGHS)

/* is p aligned to a word? */
#define __AXIOM_ALIGNED(p) (((size_t)(p) & (__AXIOM_WSIZE - 1)) == 0)

#endif
