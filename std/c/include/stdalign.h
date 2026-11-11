/* stdalign.h - alignment (C11 7.15, C23 7.15), for the Axiom compiler's C
   front end. */
#ifndef __AXIOM_STDALIGN_H
#define __AXIOM_STDALIGN_H

#if __STDC_VERSION__ < 201112L
#error "<stdalign.h> is a header of C11 and later"
#endif

/* (in C23 alignas and alignof are keywords) */
#if __STDC_VERSION__ < 202311L
#define alignas _Alignas
#define alignof _Alignof
#endif
#define __alignas_is_defined 1
#define __alignof_is_defined 1

#endif
