/* flags: --c-std=c23 */
/* expect-error: C1048 */
#if __has_c_attribute(1)
#endif
