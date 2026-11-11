/* flags: --c-std=c23 */
/* expect-error: C1083 */
#if __has_embed("inc/abc.bin" prefix(1) prefix(2))
#endif
