/* Signed overflow in #if is diagnosed. */
/* expect-error: C1033 */
#if 0x7fffffffffffffff + 1
#endif
