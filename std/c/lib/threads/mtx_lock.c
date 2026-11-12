/* mtx_lock (C17 7.26.4.3): blocks until the calling thread holds mtx (a
   recursive mutex it holds is held once more). */
#include "axc/threads_impl.h"

int mtx_lock(mtx_t *mtx) {
    return (int)__axc_mtx_lock(mtx);
}
