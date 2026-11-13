/* __axc_sig_ign: SIG_IGN on WebAssembly (std/c/include/signal.h), where a
   function pointer is a table index and 1, SIG_IGN elsewhere, is the first
   function's. Never called as a handler: an ignored signal does nothing
   (__axc_sig_deliver.c). */
#include <signal.h>

void __axc_sig_ign(int sig) {
    (void)sig;
}
