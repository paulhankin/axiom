/* mtx_init (C17 7.26.4.2): a mutex of type mtx_plain, mtx_timed,
   mtx_plain | mtx_recursive or mtx_timed | mtx_recursive; thrd_error for
   another type. */
#include "axc/threads_impl.h"

int mtx_init(mtx_t *mtx, int type) {
    return (int)__axc_mtx_init(mtx, type);
}
