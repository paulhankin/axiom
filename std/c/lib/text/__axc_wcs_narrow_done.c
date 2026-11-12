/* __axc_wcs_narrow_done - frees what __axc_wcs_narrow allocated (when p is
   not buf). */
#include <axc/text.h>

void __axc_wcs_narrow_done(char *p, char *buf) {
    if (p != buf)
        free(p);
}
