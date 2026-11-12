/* mtx_trylock (C17 7.26.4.5): takes mtx if no other thread holds it, else
   thrd_busy. */
#include "axc/threads_impl.h"

int mtx_trylock(mtx_t *mtx) {
    return (int)__axc_mtx_trylock(mtx);
}
