/* posix_memalign (POSIX; <stdlib.h> declares it in a GNU dialect): *memptr
   set to an object of size bytes aligned to alignment, a power of two that
   is a multiple of sizeof(void *) (else EINVAL), from aligned_alloc (for
   free); ENOMEM, and *memptr unchanged, if there is no memory. errno is
   left as it was. */
#include <errno.h>
#include <stdlib.h>

int posix_memalign(void **memptr, size_t alignment, size_t size) {
    int saved = errno;
    void *p;
    if (alignment == 0 || (alignment & (alignment - 1)) != 0 || alignment % sizeof(void *) != 0)
        return EINVAL;
    p = aligned_alloc(alignment, size);
    if (!p) {
        errno = saved;
        return ENOMEM;
    }
    *memptr = p;
    return 0;
}
