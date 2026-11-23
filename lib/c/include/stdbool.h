/* stdbool.h - boolean type and values (C99 7.16, C11 7.18, C23 7.19),
   for the Axiom compiler's C front end. */
#ifndef __AXIOM_STDBOOL_H
#define __AXIOM_STDBOOL_H

#if __STDC_VERSION__ < 199901L
#error "<stdbool.h> is a header of C99 and later"
#endif

/* (in C23 bool, true and false are keywords) */
#if __STDC_VERSION__ < 202311L
#define bool _Bool
#define true 1
#define false 0
#endif
#define __bool_true_false_are_defined 1

#endif
