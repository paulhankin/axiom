/* An invalid redefinition (C99 6.10.3.5 EXAMPLE 6). */
/* expect-error: C1060 */
#define OBJ_LIKE (1-1)
#define OBJ_LIKE (0)
