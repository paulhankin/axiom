/* flags: --c-std=c95 */
/* expect-error: C1012 */
/* Universal character names are C99's: before, an unknown escape sequence. */
'\u00e9'
