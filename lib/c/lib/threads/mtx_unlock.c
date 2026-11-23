/* mtx_unlock (C17 7.26.4.6): releases mtx (a recursive mutex once);
   thrd_error if it is not locked (a recursive one: by the calling
   thread). */
#include "axc/threads_impl.h"

int mtx_unlock(mtx_t *mtx) {
    return (int)__axc_mtx_unlock(mtx);
}
