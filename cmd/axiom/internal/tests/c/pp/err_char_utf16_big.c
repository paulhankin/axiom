/* A char16_t constant is one code unit. */
/* flags: --c-std=c11 */
/* expect-error: C1011 */
int c = u'😀';
