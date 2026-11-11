/* flags: --c-std=c89 */
/* expect-error: C1021 */
/* Universal character names are C99's: before, a stray backslash. */
int \u00e9;
