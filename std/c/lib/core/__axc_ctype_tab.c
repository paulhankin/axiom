/* __axc_ctype_tab: the character classes of the "C" locale (C 7.4p1,
   7.4.1, and 5.2.1 for the basic character set's members), by value + 1 of
   unsigned char, EOF at 0. Every byte past 127 is in no class. */
#include "axc/core_impl.h"

#define U __AXC_UPPER
#define L __AXC_LOWER
#define D __AXC_DIGIT
#define S __AXC_SPACE
#define P __AXC_PUNCT
#define C __AXC_CNTRL
#define X __AXC_XDIGIT
#define B __AXC_BLANK
#define R __AXC_PRINT

/* (0..127 as in ASCII, eight to a line) */
const unsigned short __axc_ctype_tab[257] = {
    0, /* EOF */
    C, C, C, C, C, C, C, C,
    C, S|C|B, S|C, S|C, S|C, S|C, C, C,
    C, C, C, C, C, C, C, C,
    C, C, C, C, C, C, C, C,
    S|B|R, P|R, P|R, P|R, P|R, P|R, P|R, P|R,
    P|R, P|R, P|R, P|R, P|R, P|R, P|R, P|R,
    D|X|R, D|X|R, D|X|R, D|X|R, D|X|R, D|X|R, D|X|R, D|X|R,
    D|X|R, D|X|R, P|R, P|R, P|R, P|R, P|R, P|R,
    P|R, U|X|R, U|X|R, U|X|R, U|X|R, U|X|R, U|X|R, U|R,
    U|R, U|R, U|R, U|R, U|R, U|R, U|R, U|R,
    U|R, U|R, U|R, U|R, U|R, U|R, U|R, U|R,
    U|R, U|R, U|R, P|R, P|R, P|R, P|R, P|R,
    P|R, L|X|R, L|X|R, L|X|R, L|X|R, L|X|R, L|X|R, L|R,
    L|R, L|R, L|R, L|R, L|R, L|R, L|R, L|R,
    L|R, L|R, L|R, L|R, L|R, L|R, L|R, L|R,
    L|R, L|R, L|R, P|R, P|R, P|R, P|R, C,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};
