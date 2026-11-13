/* ignore_handler_s (C11 K.3.6.1.3): returns, doing nothing: the function
   that found the violation then returns its failure. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdlib.h>

void ignore_handler_s(const char *__AXC_RESTRICT msg, void *__AXC_RESTRICT ptr, errno_t error) {
    (void)msg;
    (void)ptr;
    (void)error;
}
