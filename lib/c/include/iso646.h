/* iso646.h - alternative spellings (C95 7.1.1, C99 7.9, C11 7.9, C23 7.9),
   for the Axiom compiler's C front end. */
#ifndef __AXIOM_ISO646_H
#define __AXIOM_ISO646_H

#if __STDC_VERSION__ < 199409L
#error "<iso646.h> is a header of C95 and later"
#endif

#define and &&
#define and_eq &=
#define bitand &
#define bitor |
#define compl ~
#define not !
#define not_eq !=
#define or ||
#define or_eq |=
#define xor ^
#define xor_eq ^=

#endif
