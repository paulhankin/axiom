#include <stdint.h>
#ifdef COMPAT_STDINT
#error "the -idirafter directory's stdint.h was found before the system's"
#endif
int32_t x;
