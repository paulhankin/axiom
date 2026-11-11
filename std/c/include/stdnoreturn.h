/* stdnoreturn.h - _Noreturn (C11 7.23, C23 7.32), for the Axiom compiler's
   C front end. */
#ifndef __AXIOM_STDNORETURN_H
#define __AXIOM_STDNORETURN_H

#if __STDC_VERSION__ < 201112L
#error "<stdnoreturn.h> is a header of C11 and later"
#endif

#define noreturn _Noreturn

#endif
