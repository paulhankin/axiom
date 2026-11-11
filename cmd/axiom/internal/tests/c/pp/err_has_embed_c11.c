/* flags: --c-std=c11 */
/* expect-error: C1017 */
#if __has_embed("inc/abc.bin")
#endif
