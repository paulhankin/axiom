/* The C side of ffi_data.ax: objects Axiom declares, and Axiom's
   exported ones through the generated header. */
#include <stdint.h>
#include "ffi_data_generated.h"

int32_t c_count = 41;
const int64_t c_limits[3] = {100, 200, 203};
Pair c_pair = {1, 0.25};

int32_t c_touch(void) {
    ax_total += c_count;
    c_pair.b = (double)c_pair.a / 4.0 + 4.0;
    ax_pair.a += 3;
    ax_pair.b = ax_pair.b * ax_scale * 2.0;
    return c_count;
}
