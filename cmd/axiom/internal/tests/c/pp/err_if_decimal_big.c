/* C99: a decimal constant too large for intmax_t has no type. */
/* flags: --c-std=c99 */
/* expect-error: C1016 */
#if 9223372036854775808
#endif
