/* defined made by macro replacement is undefined behavior: an error. */
/* expect-error: C1044 */
#define D defined(X)
#if D
#endif
