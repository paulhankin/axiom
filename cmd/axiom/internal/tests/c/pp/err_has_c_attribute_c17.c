/* flags: --c-std=c17 */
/* expect-error: C1017 */
#if defined(X) || __has_c_attribute(nodiscard)
#endif
