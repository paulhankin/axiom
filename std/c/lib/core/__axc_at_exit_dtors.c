/* __axc_at_exit_dtors: the function that runs the program's destructors
   (the GNU dialects' destructor attribute), which exit calls after the
   atexit handlers; set by the start-up before the constructors run
   (cmd/axiom/src/c/cgen.ax, define_runners), null in a program without
   them. */
#include "axc/core_impl.h"

void (*__axc_at_exit_dtors)(void);
