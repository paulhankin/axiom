/* expect-error: C1032 */
#if 1 || 0
#elif 2 % 0
#endif
#if 0
#elif 2 % 0
#endif
