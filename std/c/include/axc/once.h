/* axc/once.h - once_flag, ONCE_FLAG_INIT and call_once, which <threads.h>
   (C11 7.26.1, 7.26.2.1) and in C23 also <stdlib.h> (7.24p3, 7.24.2.1)
   declare: here once for both (std/c/lib/threads/call_once.c). Private to
   the compiler's headers. */
#ifndef __AXIOM_AXC_ONCE_H
#define __AXIOM_AXC_ONCE_H

typedef struct {
    unsigned long long __axc_state; /* (its low 32 bits) */
} once_flag;
#define ONCE_FLAG_INIT {0}

void call_once(once_flag *__flag, void (*__func)(void));

#endif
