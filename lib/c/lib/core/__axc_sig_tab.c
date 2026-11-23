/* __axc_sig_tab: signal's handler of each of the library's signal numbers
   (SIG_DFL, SIG_IGN or a function), all SIG_DFL at start-up (C23
   7.14.1.1p6). */
#include <signal.h>
#include "axc/core_impl.h"

void (*__axc_sig_tab[32])(int);
