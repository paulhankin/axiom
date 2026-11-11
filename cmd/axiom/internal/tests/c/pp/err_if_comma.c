/* The comma operator is allowed only where it is not evaluated. */
/* expect-error: C1034 */
#if 0 && (1, 2)
#endif
#if (1, 2)
#endif
