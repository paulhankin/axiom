/* A universal character name is not one before C99: the backslash is a stray character. */
/* flags: --c-std=c89 */
/* expect-error: C1021 */
int a\u00e9;
